// Copyright 2016-2021 Google LLC. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// IDA Pro plugin to warn users if they leave their floating license idling for
// too long. It then saves the database and closes IDA.

#include <chrono>  // NOLINT
#include <cstdio>
#include <cstdlib>
#include <string>

#include <pro.h>        // NOLINT
#include <expr.hpp>     // NOLINT
#include <ida.hpp>      // NOLINT
#include <idp.hpp>      // NOLINT
#include <kernwin.hpp>  // NOLINT
#include <loader.hpp>   // NOLINT

namespace security {
namespace {

constexpr const char kPluginComment[] =
    "Prevent this instance of IDA from idling too long";
constexpr const char kPluginMenuName[] = "IDA Idle";
constexpr const char kPluginHotkey[] = "";  // No hotkey

constexpr std::chrono::seconds kTimerInterval{1};

static qtimer_t g_timer_handle{};
static bool g_ignore_activity = false;
static std::chrono::steady_clock::time_point g_last_activity{};
static std::chrono::seconds g_timer_idle_warning = std::chrono::hours{6};
static std::chrono::seconds g_timer_idle_timeout = std::chrono::hours{12};

std::string GetArgument(const char* name) {
  std::string plugin_option("IdaIdle");
  plugin_option.append(name);
  const char* option = get_plugin_options(plugin_option.c_str());
  return option ? option : "";
}

std::string HumanReadableTime(std::chrono::milliseconds msec) {
  std::string result;
  auto hours = std::chrono::duration_cast<std::chrono::hours>(msec);
  msec -= hours;
  auto minutes = std::chrono::duration_cast<std::chrono::minutes>(msec);
  msec -= minutes;
  auto seconds = std::chrono::duration_cast<std::chrono::seconds>(msec);
  msec -= seconds;
  bool need_space = false;
  if (hours.count()) {
    result += std::to_string(hours.count()) + "h";
    need_space = true;
  }
  if (minutes.count()) {
    result += (need_space ? " " : "") + std::to_string(minutes.count()) + "m";
    need_space = true;
  }
  if (seconds.count()) {
    result += (need_space ? " " : "") + std::to_string(seconds.count()) + "s";
  }
  return result;
}

void HelpIdle() {
  info(
      "This plugin displays a notification if this instance of IDA is idling "
      "for more than %s.\n"
      "After %s, it will create a database snapshot and "
      "quit without saving.",
      HumanReadableTime(g_timer_idle_warning).c_str(),
      HumanReadableTime(g_timer_idle_timeout).c_str());
}

void WarnIdle() {
  msg("IDA Idle: ATTENTION! Your session has been idle for more than %s.\n"
      "                     If you do not save your work, this plugin will "
      "create a database\n"
      "                     snapshot and close IDA without saving.\n"
      "                     This will happen in %s.\n",
      HumanReadableTime(g_timer_idle_warning).c_str(),
      HumanReadableTime(g_timer_idle_timeout - g_timer_idle_warning).c_str());
}

void CreateSnapshotAndQuit() {
  const auto* database_idb = get_path(PATH_TYPE_IDB);
  if (strlen(database_idb) > 0) {
    msg("IDA Idle: Saving snapshot...\n");
    snapshot_t snapshot;
    std::snprintf(snapshot.desc, MAX_DATABASE_DESCRIPTION,
                  "IDA Idle auto snapshot");
    qstring error;
    if (!take_database_snapshot(&snapshot, &error)) {
      warning("IDA Idle: Could not take a database snapshot: %s",
              error.c_str());
      return;
    }
    msg("IDA Idle: Saved snapshot to: %s\n", snapshot.filename);

    // Set the temp flag so that if IDA quits the unpacked files are deleted.
    set_database_flag(DBFL_TEMP);
  }

  msg("IDA Idle: Closing IDA...\n");

  g_ignore_activity = true;

  class QuitRequest : public ui_request_t {
    bool idaapi run() override {
      // Cannot use process_ui_action("Quit") here, since that unloads the
      // plugin even before returning and will thus crash in invalid memory.
      qexit(0);
    }
  };
  execute_ui_requests(new QuitRequest() /* Takes ownership */);
}

int OnTimer(void* /* user_data */) {
  auto diff = std::chrono::duration_cast<std::chrono::seconds>(
      std::chrono::steady_clock::now() - g_last_activity);
  g_ignore_activity = true;
  if (diff > g_timer_idle_timeout) {
    CreateSnapshotAndQuit();
    return -1;  // Cancel timer, continue to ignore UI events.
  }
  if (diff > g_timer_idle_warning &&
      diff <= g_timer_idle_warning + kTimerInterval) {
    WarnIdle();
  }
  g_ignore_activity = false;
  return std::chrono::milliseconds(kTimerInterval).count();
}

ssize_t idaapi OnUiNotification(void* /* user_data */,
                                int /* notification_code */,
                                va_list /* args */) {
  if (!g_ignore_activity) {
    // Every time there's UI activity, reset the idle time.
    g_last_activity = std::chrono::steady_clock::now();
  }
  return 0;
}

int idaapi PluginInit() {
  const auto default_warning_seconds = g_timer_idle_warning.count();
  int warning_seconds = std::atoi(GetArgument("WarningSeconds").c_str());
  if (warning_seconds == 0) {
    warning_seconds = default_warning_seconds;
  }
  const auto default_timeout_seconds = g_timer_idle_timeout.count();
  int timeout_seconds =  std::atoi(GetArgument("TimeoutSeconds").c_str());
  if (timeout_seconds == 0) {
    timeout_seconds = default_timeout_seconds;
  }

  if (warning_seconds < timeout_seconds) {
    if (warning_seconds != default_warning_seconds) {
      g_timer_idle_warning = std::chrono::seconds(warning_seconds);
      msg("IDA Idle: Warning interval set to %s via plugin option\n",
          HumanReadableTime(g_timer_idle_warning).c_str());
    }
    if (timeout_seconds != default_timeout_seconds) {
      g_timer_idle_timeout = std::chrono::seconds(timeout_seconds);
      msg("IDA Idle: Timeout interval set to %s via plugin option\n",
          HumanReadableTime(g_timer_idle_timeout).c_str());
    }
  } else {
    msg("IDA Idle: Timeout smaller or equal to warning interval, both "
        "ignored\n");
  }

  addon_info_t addon_info;
  addon_info.cb = sizeof(addon_info_t);
  addon_info.id = "com.google.idaidle";
  addon_info.name = "IDA Idle";
  addon_info.producer = "Google";
  addon_info.version = "0.6";
  addon_info.freeform = "(c)2016-2021 Google LLC";
  register_addon(&addon_info);

  g_timer_handle =
      register_timer(std::chrono::milliseconds(kTimerInterval).count(),
                     &OnTimer, /*ud=*/nullptr);
  if (!hook_to_notification_point(HT_UI, OnUiNotification,
                                  /*user_data=*/nullptr)) {
    msg("IDA Idle: Failed to register plugin notifications, skipping plugin");
    return PLUGIN_SKIP;
  }
  return PLUGIN_KEEP;
}

bool idaapi PluginRun(size_t /* arg */) {
  HelpIdle();
  return true;
}

void idaapi PluginTerminate() {
  unregister_timer(g_timer_handle);
  unhook_from_notification_point(HT_UI, OnUiNotification,
                                 /*user_data=*/nullptr);
}

} // namespace
} // namespace security

extern "C" {
plugin_t PLUGIN{
    IDP_INTERFACE_VERSION,
    PLUGIN_FIX,                 // Plugin flags
    security::PluginInit,       // Initialize
    security::PluginTerminate,  // Terminate
    security::PluginRun,        // Invoke plugin
    security::kPluginComment,   // Statusline text
    nullptr,                    // Multi-line help about the plugin, unused
    security::kPluginMenuName,  // Preferred short name of the plugin
    security::kPluginHotkey     // Preferred hotkey to run the plugin
};
}

#pragma once

#include <SDKDDKVer.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <thread>

#include <shellapi.h>
#include <Resource.h>

#define ID_TRAY_ICON 0x1234
#define ID_TRAY_EXIT_CONTEXT_MENU_ITEM 0x1236

#define WM_EX_MESSAGE WM_APP+0x1000

#include <ControlServer.hpp>
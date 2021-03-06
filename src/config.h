#pragma once

namespace config
{
    const float speed_region[4]                = {234.0 / 300, 199.0 / 300, 32.0 / 2000, 5.0 / 200};
    const float speed_limit_region[4]          = {235.5 / 300, 247.5 / 300, 26.0 / 2000, 4.0 / 200};
    const float cruise_control_speed_region[4] = {1485.0 / 3000, 210.0 / 300, 22.0 / 2000, 5.0 / 400};
    const float drive_window_region[4]         = {600.0 / 1920, 280.0 / 1080, 1000.0 / 1920, 360.0 / 1080};
    const float map_region[4]                  = {1626.0 / 1920, 800.0 / 1080, 120.0 / 1920, 120.0 / 1080};
    const float right_mirror_region[4]         = {1620.0 / 1920, 90.0 / 1080, 275.0 / 1920, 360.0 / 1080};
    const float left_mirror_region[4]          = {30.0 / 1920, 320.0 / 1080, 240.0 / 1920, 300.0 / 1080};
}
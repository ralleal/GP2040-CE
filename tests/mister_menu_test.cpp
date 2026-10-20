#include "mister_menu_production.h"
#include <initializer_list>

static unsigned checks = 0;
static const char* scenario;

struct Fixture {
    KeyboardDriver driver;
    Gamepad gamepad{false};
    uint32_t origin;

    explicit Fixture(uint32_t start = 0) : origin(start) {}

    void sample(uint32_t time, bool down, std::initializer_list<uint8_t> expected = {}) {
        clockMs = origin + time;
        gamepad.down = down;
        driver.process(&gamepad);
        ++checks;
        if (driver.keys != std::set<uint8_t>(expected)) {
            std::fprintf(stderr, "%s at %u ms: unexpected keys (origin %u)\n", scenario, time, origin);
            std::exit(1);
        }
    }

    // A debounced first short click, logical release at t=130.
    void firstClick() {
        sample(0, true);
        sample(29, true);
        sample(30, true);
        sample(100, false);
        sample(129, false);
        sample(130, false);
    }
};

static void singleAndPulse(uint32_t origin) {
    scenario = "single click and 200 ms pulse";
    Fixture f(origin);
    f.firstClick();
    f.sample(429, false);
    f.sample(430, false, {HID_KEY_F13});
    f.sample(629, false, {HID_KEY_F13});
    f.sample(630, false);
    f.sample(1000, false);
}

static void doubleBoundary(uint32_t origin, uint32_t gap, bool stalled) {
    scenario = stalled ? "second click after process stall" : "second click window boundary";
    Fixture f(origin);
    f.firstClick();
    const uint32_t start = 130 + gap;
    if (!stalled) f.sample(gap < 300 ? start - 1 : 429, false);
    const bool valid = gap < 300;
    if (valid) {
        f.sample(start, true);
        f.sample(start + 29, true);
        f.sample(start + 30, true);
        f.sample(start + 100, false);
        f.sample(start + 130, false, {HID_KEY_F12});
        f.sample(start + 209, false, {HID_KEY_F12});
        f.sample(start + 210, false);
        f.sample(start + 1000, false);
    } else {
        // An overdue first click must become F13, even if raw A1 is down.
        f.sample(start, true, {HID_KEY_F13});
        f.sample(start + 30, true, {HID_KEY_F13});
        f.sample(start + 100, false, {HID_KEY_F13});
        f.sample(start + 130, false, {HID_KEY_F13});
        f.sample(start + 200, false);
        f.sample(start + 429, false);
        f.sample(start + 430, false, {HID_KEY_F13});
        f.sample(start + 630, false);
    }
}

static void hold(uint32_t origin, uint32_t duration, bool second, bool skipThreshold) {
    scenario = second ? "first short click plus second hold" : "hold boundaries";
    Fixture f(origin);
    uint32_t start = 0;
    if (second) {
        f.firstClick();
        start = 200;
    }
    f.sample(start, true);
    f.sample(start + 30, true);
    const uint32_t threshold = start + 30 + 2000;
    skipThreshold = skipThreshold || duration < 2030;
    if (duration >= 2000 && !skipThreshold) {
        f.sample(threshold - 1, true);
        f.sample(threshold, true, {HID_KEY_F14, HID_KEY_F15, HID_KEY_F16, HID_KEY_F17});
    }
    const uint32_t release = start + duration;
    if (duration >= 2000 && release >= threshold) {
        f.sample(release, false, {HID_KEY_F14, HID_KEY_F15, HID_KEY_F16, HID_KEY_F17});
    } else {
        f.sample(release, false);
    }
    const uint32_t end = release + 30;
    if (duration >= 2000) {
        f.sample(end, false, {HID_KEY_F14, HID_KEY_F15, HID_KEY_F16, HID_KEY_F17});
        const uint32_t pulse = release < threshold ? end : (skipThreshold ? release : threshold);
        f.sample(pulse + 499, false, {HID_KEY_F14, HID_KEY_F15, HID_KEY_F16, HID_KEY_F17});
        f.sample(pulse + 500, false);
    } else if (duration <= 500) {
        if (second) {
            f.sample(end, false, {HID_KEY_F12});
            f.sample(end + 80, false);
        } else {
            f.sample(end, false);
            f.sample(end + 300, false, {HID_KEY_F13});
            f.sample(end + 500, false);
        }
    } else {
        f.sample(end, false);
        f.sample(end + 300, false);
    }
    f.sample(end + 2000, false);
}

static void bounce(uint32_t origin) {
    scenario = "bounce cannot complete a click";
    Fixture f(origin);
    f.sample(0, true);
    f.sample(29, false);
    f.sample(60, false);
    f.sample(100, true);
    f.sample(110, false);
    f.sample(120, true);
    f.sample(149, true);
    f.sample(150, true);
    f.sample(200, false);
    f.sample(210, true);
    f.sample(220, false);
    f.sample(250, false); // first debounced release
    f.sample(549, false);
    f.sample(550, false, {HID_KEY_F13});
    f.sample(750, false);

    scenario = "in-window noise followed by late stable press";
    Fixture g(origin);
    g.firstClick();
    g.sample(429, true);
    g.sample(440, false, {HID_KEY_F13});
    g.sample(441, true, {HID_KEY_F13});
    g.sample(471, true, {HID_KEY_F13});
    g.sample(541, false, {HID_KEY_F13});
    g.sample(571, false, {HID_KEY_F13});
    g.sample(640, false);
    g.sample(871, false, {HID_KEY_F13});
    g.sample(1071, false);
}

static void delayedDebounce(uint32_t origin) {
    scenario = "in-window raw press debounced after deadline and process stall";
    Fixture f(origin);
    f.firstClick();
    f.sample(429, true); // 299 ms: do not emit the single at expiry
    f.sample(430, true);
    f.sample(500, true); // first debounced observation is well after expiry
    f.sample(510, false);
    f.sample(540, false, {HID_KEY_F12});
    f.sample(620, false);

    scenario = "aborted second click eventually emits first single";
    Fixture g(origin);
    g.firstClick();
    g.sample(429, true);
    g.sample(450, false, {HID_KEY_F13});
    g.sample(650, false);
}

static void longOnce(uint32_t origin) {
    scenario = "long action fires once and next click starts fresh";
    Fixture f(origin);
    f.sample(0, true);
    f.sample(30, true);
    f.sample(2030, true, {HID_KEY_F14, HID_KEY_F15, HID_KEY_F16, HID_KEY_F17});
    f.sample(2529, true, {HID_KEY_F14, HID_KEY_F15, HID_KEY_F16, HID_KEY_F17});
    f.sample(2530, true);
    f.sample(4000, true);
    f.sample(4100, false);
    f.sample(4130, false);
    f.sample(4200, true);
    f.sample(4230, true);
    f.sample(4300, false);
    f.sample(4330, false);
    f.sample(4630, false, {HID_KEY_F13});
}

int main() {
    // Repeat across the uint32_t clock wrap, including a window that straddles it.
    for (uint32_t origin : {0u, UINT32_MAX - 200u, UINT32_MAX - 450u, UINT32_MAX - 2100u}) {
        singleAndPulse(origin);
        for (uint32_t gap : {269u, 270u, 271u, 299u, 300u, 301u, 350u}) {
            doubleBoundary(origin, gap, false);
            doubleBoundary(origin, gap, true);
        }
        for (uint32_t duration : {30u, 499u, 500u, 501u, 1999u, 2000u, 2001u, 2100u}) {
            for (bool second : {false, true}) {
                hold(origin, duration, second, false);
                if (duration >= 2000) hold(origin, duration, second, true);
            }
        }
        bounce(origin);
        delayedDebounce(origin);
        longOnce(origin);
    }
    std::printf("MiSTer A1 regression tests passed (%u checks).\n", checks);
}

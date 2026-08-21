// cursor-core — "freeze cursor after dragging" state machine (slice 004-07).
//
// Pure C++17, no ImGui / Windows / clock: the DLL feeds it live input each frame
// (freeze toggle, mouse-button state, pointer, frame dt) and draws the marker at
// the position it returns. Keeping the state machine here makes the drag/freeze
// logic unit-testable off-game with synthetic (button, pointer, dt) sequences.
//
// Behaviour (mockup: "Hold the overlay in place when you release a drag"):
//   * A *drag* is a mouse-button hold that either lasts longer than a click
//     (kDragHoldSeconds) or moves the pointer past kDragMoveThresholdPx — this
//     distinguishes a drag from a plain click, and (via the duration arm) still
//     fires when GW2 centre-locks the pointer during mouse-look (deltas ~0).
//   * On the drag's release the marker FREEZES at the release position.
//   * The freeze clears as soon as the pointer next moves past kResumeThresholdPx,
//     and normal tracking resumes.
//   * With the toggle off the machine is a pass-through (returns the live pointer)
//     and holds no state.
#pragma once

namespace cursor {

// A screen-space point in pixels. Plain floats — deliberately not ImVec2, to keep
// ImGui out of cursor-core (the DLL converts at the draw site).
struct Vec2 {
    float x = 0.0f;
    float y = 0.0f;
};

// Named thresholds so an in-game correction is a one-line change (A1/A2).
inline constexpr float kDragMoveThresholdPx = 4.0f;  // motion while held => a drag
inline constexpr float kDragHoldSeconds     = 0.18f; // held longer than a click => a drag
inline constexpr float kResumeThresholdPx   = 4.0f;  // motion that ends a freeze

class DragFreeze {
public:
    // Advance one frame; returns where the marker should be drawn this frame.
    //   enabled     — the freeze_after_drag toggle
    //   button_down — any drag-capable mouse button currently held
    //   pointer     — the live pointer (OS cursor hotspot) this frame
    //   dt_seconds  — frame delta time (>= 0)
    Vec2 update(bool enabled, bool button_down, Vec2 pointer, float dt_seconds);

    bool frozen() const { return frozen_; }

    // Clear all state (called when the toggle is off / the marker is disabled).
    void reset();

private:
    bool  have_prev_ = false;
    bool  prev_down_ = false;

    // Current hold classification.
    bool  in_hold_      = false;
    bool  is_drag_      = false;
    float hold_seconds_ = 0.0f;
    Vec2  hold_start_{};

    // Freeze state.
    bool  frozen_ = false;
    Vec2  frozen_pos_{};
};

} // namespace cursor

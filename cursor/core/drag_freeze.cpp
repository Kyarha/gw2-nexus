#include "core/drag_freeze.h"

namespace cursor {

namespace {

// Squared distance — avoids a sqrt; thresholds are squared at the comparison.
float dist_sq(Vec2 a, Vec2 b)
{
    const float dx = a.x - b.x;
    const float dy = a.y - b.y;
    return dx * dx + dy * dy;
}

} // namespace

void DragFreeze::reset()
{
    have_prev_    = false;
    prev_down_    = false;
    in_hold_      = false;
    is_drag_      = false;
    hold_seconds_ = 0.0f;
    frozen_       = false;
}

Vec2 DragFreeze::update(bool enabled, bool button_down, Vec2 pointer, float dt_seconds)
{
    // Toggle off: pass through the live pointer and hold no state, so turning the
    // feature off can never leave the marker stuck (AC4).
    if (!enabled)
    {
        reset();
        return pointer;
    }

    const float move_sq   = kDragMoveThresholdPx * kDragMoveThresholdPx;
    const float resume_sq  = kResumeThresholdPx * kResumeThresholdPx;

    // Classify the current hold: a drag if it lasts longer than a click OR the
    // pointer moves past the threshold while held (either arm latches is_drag_).
    if (button_down)
    {
        if (!in_hold_)
        {
            in_hold_      = true;
            is_drag_      = false;
            hold_seconds_ = 0.0f;
            hold_start_   = pointer;
        }
        hold_seconds_ += (dt_seconds > 0.0f ? dt_seconds : 0.0f);
        if (hold_seconds_ >= kDragHoldSeconds) { is_drag_ = true; }
        if (dist_sq(pointer, hold_start_) >= move_sq) { is_drag_ = true; }
    }

    // Release edge (down -> up): if the hold was a drag, freeze at the release
    // position. A plain click (short, still) is not a drag and does not freeze.
    const bool released = have_prev_ && prev_down_ && !button_down;
    if (released)
    {
        if (is_drag_)
        {
            frozen_     = true;
            frozen_pos_ = pointer;
        }
        in_hold_      = false;
        is_drag_      = false;
        hold_seconds_ = 0.0f;
    }
    if (!button_down) { in_hold_ = false; }

    // Resume: any pointer movement past the threshold ends the freeze. On the
    // release frame frozen_pos_ == pointer, so the freeze survives to next frames.
    if (frozen_ && dist_sq(pointer, frozen_pos_) >= resume_sq)
    {
        frozen_ = false;
    }

    have_prev_ = true;
    prev_down_ = button_down;

    return frozen_ ? frozen_pos_ : pointer;
}

} // namespace cursor

#ifndef IME_STATE_H
#define IME_STATE_H

struct ImeState {
    enum State {
        STATE_BYPASS, STATE_IDLE, STATE_INPUT, STATE_COMPOSING, STATE_PREDICT,
        STATE_APP_COMPLETION
    };
};

#endif // IME_STATE_H

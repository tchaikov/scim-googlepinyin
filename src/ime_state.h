/*
 * Copyright (C) 2009 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef IME_STATE_H
#define IME_STATE_H

struct ImeState {
    enum State {
        STATE_BYPASS, STATE_IDLE, STATE_INPUT, STATE_COMPOSING, STATE_PREDICT,
        STATE_APP_COMPLETION
    };
};

#endif // IME_STATE_H

#ifndef COMPOSING_VIEW_H
#define COMPOSING_VIEW_H


class GooglePyInstance;

/**
 * View used to show composing string (The Pinyin string for the unselected
 * syllables and the Chinese string for the selected syllables.)
 */
class ComposingView
{
    
public:
        /**
     * <p>
     * There are three statuses for the composing view.
     * </p>
     * 
     * <p>
     * {@link #SHOW_PINYIN} is used to show the current Pinyin string without
     * highlighted effect. When user inputs Pinyin characters one by one, the
     * Pinyin string will be shown in this mode.
     * </p>
     * <p>
     * {@link #SHOW_STRING_LOWERCASE} is used to show the Pinyin string in
     * lowercase with highlighted effect. When user presses UP key and there is
     * no fixed Chinese characters, composing view will switch from
     * {@link #SHOW_PINYIN} to this mode, and in this mode, user can press
     * confirm key to input the lower-case string, so that user can input
     * English letter in Chinese mode.
     * </p>
     * <p>
     * {@link #EDIT_PINYIN} is used to edit the Pinyin string (shown with
     * highlighted effect). When current status is {@link #SHOW_PINYIN} and user
     * presses UP key, if there are fixed Characters, the input method will
     * switch to {@link #EDIT_PINYIN} thus user can modify some characters in
     * the middle of the Pinyin string. If the current status is
     * {@link #SHOW_STRING_LOWERCASE} and user presses LEFT and RIGHT key, it
     * will also switch to {@link #EDIT_PINYIN}.
     * </p>
     * <p>
     * Whenever user presses down key, the status switches to
     * {@link #SHOW_PINYIN}.
     * </p>
     * <p>
     * When composing view's status is {@link #SHOW_PINYIN}, the IME's status is
     * {@link PinyinIME.ImeState#STATE_INPUT}, otherwise, the IME's status
     * should be {@link PinyinIME.ImeState#STATE_COMPOSING}.
     * </p>
     */
    enum Status {
        SHOW_PINYIN, SHOW_STRING_LOWERCASE, EDIT_PINYIN,
    };
    Status get_status() const;
    void reset();
    void set_visibility(bool visible);
    void move_cursor(int offset);
    
private:
    Status m_status;
    GooglePyInstance *m_pinyin;
};

#endif // COMPOSING_VIEW_H

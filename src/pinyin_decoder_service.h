#include <list>
#include <vector>
#include <string>

using std::vector;
using std::list;
using std::wstring;
using std::string;


class PinyinDecoderService
{
public:
    size_t search(const string& spelling_str);
    void reset_search();
    size_t del_search(size_t pos, bool is_pos_in_splid, bool clear_fixed_this_step);
    int choose(int cand_id);
    vector<int> get_spelling_start();
    wstring get_predict_item(int predict_no);
    wstring get_choice(size_t cand_id);
    std::list<wstring> get_predict_list(int predicts_start,
                                        int predicts_num);
    wstring get_choice(int index);
    size_t get_fixed_len();

    string get_py_str(bool decoded);
    
    std::list<wstring> get_choice_list(int choices_start, int choices_num,
                                       int sent_fixed_len);
};

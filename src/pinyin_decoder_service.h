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
    PinyinDecoderService(const string& sys_dict_path,
                         const string& usr_dict_path);
    ~PinyinDecoderService();
    bool is_initialized() const;
    
public:
    size_t search(const string& spelling_str);
    void reset_search();
    size_t del_search(size_t pos, bool is_pos_in_splid, bool clear_fixed_this_step);
    
    int choose(int cand_id);
    wstring get_choice(size_t cand_id) const;
    std::list<wstring> get_choice_list(int choices_start, int choices_num,
                                       int sent_fixed_len) const;
    wstring get_predict_item(int predict_no) const;
    std::list<wstring> get_predict_list(int predicts_start,
                                        int predicts_num) const;
    vector<int> get_spelling_start();
    size_t get_fixed_len() const;
    string get_py_str(bool decoded);
    
private:
    void init_pinyin_engine();
    void fini_pinyin_engine();

private:
    bool m_inited;
    string m_sys_dict;
    string m_usr_dict;
};

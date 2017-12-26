
#ifndef FOREACH_ACTION

#define FOREACH_ACTION(X, Y)

#endif

// convert key
FOREACH_ACTION(CONVERT, action_convert);
FOREACH_ACTION(PREDICT, action_predict);

// candidates keys
FOREACH_ACTION(CANDIDATES_PAGE_UP, action_candidates_page_up);
FOREACH_ACTION(CANDIDATES_PAGE_DOWN, action_candidates_page_down);
FOREACH_ACTION(SELECT_FIRST_CANDIDATE, action_select_first_candidate);
FOREACH_ACTION(SELECT_LAST_CANDIDATE, action_select_last_candidate);
FOREACH_ACTION(SELECT_NEXT_CANDIDATE, action_select_next_candidate);
FOREACH_ACTION(SELECT_PREV_CANDIDATE, action_select_prev_candidate);

// segment keys
FOREACH_ACTION(SELECT_FIRST_SEGMENT, action_select_first_segment);
FOREACH_ACTION(SELECT_LAST_SEGMENT, action_select_last_segment);
FOREACH_ACTION(SELECT_NEXT_SEGMENT, action_select_next_segment);
FOREACH_ACTION(SELECT_PREV_SEGMENT, action_select_prev_segment);
FOREACH_ACTION(SHRINK_SEGMENT, action_shrink_segment);
FOREACH_ACTION(EXPAND_SEGMENT, action_expand_segment);
FOREACH_ACTION(COMMIT_FIRST_SEGMENT, action_commit_first_segment);
FOREACH_ACTION(COMMIT_SELECTED_SEGMENT, action_commit_selected_segment);
FOREACH_ACTION(COMMIT_FIRST_SEGMENT_REVERSE_LEARN,
               action_commit_first_segment_reverse_preference);
FOREACH_ACTION(COMMIT_SELECTED_SEGMENT_REVERSE_LEARN,
               action_commit_selected_segment_reverse_preference);

// direct convert keys
FOREACH_ACTION(CONV_CHAR_TYPE_FORWARD, action_convert_char_type_forward);
FOREACH_ACTION(CONV_CHAR_TYPE_BACKWARD, action_convert_char_type_backward);
FOREACH_ACTION(CONV_TO_HIRAGANA, action_convert_to_hiragana);
FOREACH_ACTION(CONV_TO_KATAKANA, action_convert_to_katakana);
FOREACH_ACTION(CONV_TO_HALF, action_convert_to_half);
FOREACH_ACTION(CONV_TO_HALF_KATAKANA, action_convert_to_half_katakana);
FOREACH_ACTION(CONV_TO_LATIN, action_convert_to_latin);
FOREACH_ACTION(CONV_TO_WIDE_LATIN, action_convert_to_wide_latin);

// pseudo ascii mode
FOREACH_ACTION(CANCEL_PSEUDO_ASCII_MODE, action_cancel_pseudo_ascii_mode);

// caret keys
FOREACH_ACTION(MOVE_CARET_FIRST, action_move_caret_first);
FOREACH_ACTION(MOVE_CARET_LAST, action_move_caret_last);
FOREACH_ACTION(MOVE_CARET_FORWARD, action_move_caret_forward);
FOREACH_ACTION(MOVE_CARET_BACKWARD, action_move_caret_backward);

// edit keys
FOREACH_ACTION(BACKSPACE, action_back);
FOREACH_ACTION(DELETE, action_delete);
FOREACH_ACTION(COMMIT, action_commit_follow_preference);
FOREACH_ACTION(COMMIT_REVERSE_LEARN, action_commit_reverse_preference);
FOREACH_ACTION(CANCEL, action_revert);
FOREACH_ACTION(CANCEL_ALL, action_cancel_all);
FOREACH_ACTION(INSERT_SPACE, action_insert_space);
FOREACH_ACTION(INSERT_ALT_SPACE, action_insert_alternative_space);
FOREACH_ACTION(INSERT_HALF_SPACE, action_insert_half_space);
FOREACH_ACTION(INSERT_WIDE_SPACE, action_insert_wide_space);

// mode keys
FOREACH_ACTION(CIRCLE_INPUT_MODE, action_circle_input_mode);
FOREACH_ACTION(CIRCLE_KANA_MODE, action_circle_kana_mode);
FOREACH_ACTION(CIRCLE_LATIN_HIRAGANA_MODE, action_circle_latin_hiragana_mode);
FOREACH_ACTION(CIRCLE_TYPING_METHOD, action_circle_typing_method);
FOREACH_ACTION(LATIN_MODE, action_latin_mode);
FOREACH_ACTION(WIDE_LATIN_MODE, action_wide_latin_mode);
FOREACH_ACTION(HIRAGANA_MODE, action_hiragana_mode);
FOREACH_ACTION(KATAKANA_MODE, action_katakana_mode);
FOREACH_ACTION(HALF_KATAKANA_MODE, action_half_katakana_mode);

// dict keys
FOREACH_ACTION(DICT_ADMIN, action_launch_dict_admin_tool);
FOREACH_ACTION(ADD_WORD, action_add_word);

// reconvert
FOREACH_ACTION(RECONVERT, action_reconvert);

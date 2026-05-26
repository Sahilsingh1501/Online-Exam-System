#ifndef EXAM_H
#define EXAM_H

#define MAX_TEXT_LEN  256
#define MAX_OPTIONS   4
#define MAX_OPT_LEN   128
#define MAX_NAME_LEN  50

// 1. Union for representing multiple types of question answers
union Answer {
    char mcq_choice;  // MCQ: 'A', 'B', 'C', or 'D'
    int tf_choice;    // True/False: 1 = True, 0 = False
};

// 2. Structure for Question
typedef struct {
    int id;
    char text[MAX_TEXT_LEN];
    int type; // 0 = Multiple Choice, 1 = True/False
    char options[MAX_OPTIONS][MAX_OPT_LEN]; // 2D array for MCQ options
    union Answer correct_ans;
} Question;

// 3. Structure for User Account
typedef struct {
    char username[MAX_NAME_LEN];
    char password[MAX_NAME_LEN];
    int is_admin; // 1 = Admin, 0 = Student
} User;

// 4. Structure for Exam Result
typedef struct {
    char username[MAX_NAME_LEN];
    int score;
    int total_questions;
    float percentage;
} ExamResult;

// Function Prototypes

// File Handling Functions (using pointers & references)
int load_questions(Question **questions);
void save_questions(Question *questions, int count);
int load_results(ExamResult **results);
void save_result(const ExamResult *result);
int register_user(const User *new_user);
int authenticate_user(const char *username, const char *password, User *user);
void seed_initial_data();

// Dynamic Question Management Functions (Admin)
void add_question();
void view_all_questions();
void delete_question();
void view_exam_statistics();

// Exam Session Functions (Student)
void take_exam(const User *student);
void view_my_results(const User *student);

// Recursive Algorithms
int recursive_binary_search(Question *questions, int low, int high, int target_id);
void recursive_calculate_stats(ExamResult *results, int index, int count, float *sum_percentage, int *pass_count, float *max_percentage);

// Input Validation and UI helpers
void clear_screen();
void press_any_key();
void print_banner(const char *title);
void show_loading(const char *msg);
void remove_newline(char *str);
void get_masked_password(char *password, int max_len);

#endif // EXAM_H

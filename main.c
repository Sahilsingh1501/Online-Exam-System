#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "exam.h"

// ============================================================
//  SECTION 1: UTILITY & INPUT HELPERS
// ============================================================

// Remove trailing newline and carriage returns from string
void remove_newline(char *str) {
    int i = 0;
    while (str[i] != '\0') {
        if (str[i] == '\n' || str[i] == '\r') {
            str[i] = '\0';
            break;
        }
        i++;
    }
}

// Password masking: reads chars one by one and echoes '*'
void get_masked_password(char *password, int max_len) {
    int i = 0;
    int ch;
    // Flush any leftover newline/carriage return left by a previous scanf call
    while ((ch = getchar()) == '\n' || ch == '\r');
    // ch now holds the first real character of the password (or EOF)
    if (ch != EOF) {
        password[i++] = (char)ch;
        printf("*");
    }
    // Read the rest of the password
    while (1) {
        ch = getchar();
        if (ch == '\r' || ch == '\n' || ch == EOF) {
            break;
        } else if (ch == '\b' || ch == 127) { // Backspace
            if (i > 0) {
                i--;
                printf("\b \b");
            }
        } else if (i < max_len - 1) {
            password[i++] = (char)ch;
            printf("*");
        }
    }
    password[i] = '\0';
    printf("\n");
}

// UI helper animations
void show_loading(const char *msg) {
    printf("%s...\n", msg);
}

void clear_screen() {
    // Uses ANSI escape code to clear the terminal screen
    printf("\033[H\033[2J");
    fflush(stdout);
}

void press_any_key() {
    printf("\nPress any key to return to menu...");
    // Flush any leftover input then wait for one keypress
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
    getchar();
}

// Convert a string to an integer (alternative to atoi)
int string_to_int(const char *str) {
    int result = 0;
    int i = 0;
    while (str[i] == ' ' || str[i] == '\t' || str[i] == '\n' || str[i] == '\r') {
        i++;
    }
    while (str[i] >= '0' && str[i] <= '9') {
        result = result * 10 + (str[i] - '0');
        i++;
    }
    return result;
}

void print_banner(const char *title) {
    clear_screen();
    printf("\n*** ONLINE EXAMINATION CONTROLLER ***\n");
    if (title && strlen(title) > 0) {
        printf(">> %s <<\n\n", title);
    }
}

// ============================================================
//  SECTION 2: FILE HANDLING — QUESTIONS DATABASE
// ============================================================

// Dynamic Question database operations (Text File Handling & Double Pointers)
int load_questions(Question **questions) {
    FILE *file = fopen("questions.txt", "r");
    if (!file) {
        *questions = NULL;
        return 0;
    }
    
    int capacity = 10;
    int count = 0;
    *questions = (Question *)malloc(capacity * sizeof(Question));
    if (*questions == NULL) {
        fclose(file);
        return 0;
    }
    
    char temp[512];
    while (fgets(temp, sizeof(temp), file)) {
        if (count >= capacity) {
            capacity *= 2;
            Question *t = (Question *)realloc(*questions, capacity * sizeof(Question));
            if (!t) {
                fclose(file);
                return count;
            }
            *questions = t;
        }
        
        Question q;
        q.id = string_to_int(temp);
        
        if (!fgets(temp, sizeof(temp), file)) break;
        q.type = string_to_int(temp);
        
        if (!fgets(q.text, sizeof(q.text), file)) break;
        remove_newline(q.text);
        
        if (q.type == 0) {
            for (int i = 0; i < 4; i++) {
                if (!fgets(q.options[i], sizeof(q.options[i]), file)) break;
                remove_newline(q.options[i]);
            }
            if (!fgets(temp, sizeof(temp), file)) break;
            q.correct_ans.mcq_choice = temp[0];
        } else {
            if (!fgets(temp, sizeof(temp), file)) break;
            q.correct_ans.tf_choice = string_to_int(temp);
        }
        
        (*questions)[count] = q;
        count++;
    }
    
    fclose(file);
    return count;
}

void save_questions(Question *questions, int count) {
    FILE *file = fopen("questions.txt", "w");
    if (!file) {
        printf("Error saving questions to database.\n");
        return;
    }
    for (int i = 0; i < count; i++) {
        fprintf(file, "%d\n", questions[i].id);
        fprintf(file, "%d\n", questions[i].type);
        fprintf(file, "%s\n", questions[i].text);
        if (questions[i].type == 0) {
            for (int j = 0; j < MAX_OPTIONS; j++) {
                fprintf(file, "%s\n", questions[i].options[j]);
            }
            fprintf(file, "%c\n", questions[i].correct_ans.mcq_choice);
        } else {
            fprintf(file, "%d\n", questions[i].correct_ans.tf_choice);
        }
    }
    fclose(file);
}

// ============================================================
//  SECTION 3: FILE HANDLING — RESULTS DATABASE
// ============================================================

// Result management: loads records from text file (CSV format) using realloc pointers
int load_results(ExamResult **results) {
    FILE *file = fopen("results.txt", "r");
    if (!file) {
        *results = NULL;
        return 0;
    }
    
    int capacity = 10;
    int count = 0;
    *results = (ExamResult *)malloc(capacity * sizeof(ExamResult));
    if (*results == NULL) {
        fclose(file);
        return 0;
    }
    
    char line[256];
    while (fgets(line, sizeof(line), file)) {
        if (count >= capacity) {
            capacity *= 2;
            ExamResult *temp = (ExamResult *)realloc(*results, capacity * sizeof(ExamResult));
            if (!temp) {
                // If realloc fails, return what we have so far
                fclose(file);
                return count;
            }
            *results = temp;
        }
        
        ExamResult r;
        // Text parsing using sscanf: username,score,total_questions,percentage
        if (sscanf(line, "%[^,],%d,%d,%f", r.username, &r.score, &r.total_questions, &r.percentage) == 4) {
            (*results)[count] = r;
            count++;
        }
    }
    fclose(file);
    return count;
}

void save_result(const ExamResult *result) {
    FILE *file = fopen("results.txt", "a");
    if (!file) {
        printf("Error: Could not record exam outcome.\n");
        return;
    }
    fprintf(file, "%s,%d,%d,%.2f\n", result->username, result->score, result->total_questions, result->percentage);
    fclose(file);
}

// ============================================================
//  SECTION 4: FILE HANDLING — USER ACCOUNTS
// ============================================================

int register_user(const User *new_user) {
    FILE *file = fopen("users.txt", "r");
    if (file) {
        char line[256];
        while (fgets(line, sizeof(line), file)) {
            User u;
            if (sscanf(line, "%[^,],%[^,],%d", u.username, u.password, &u.is_admin) == 3) {
                if (strcmp(u.username, new_user->username) == 0) {
                    fclose(file);
                    return 0; // Username already taken
                }
            }
        }
        fclose(file);
    }
    
    file = fopen("users.txt", "a");
    if (!file) {
        return -1;
    }
    fprintf(file, "%s,%s,%d\n", new_user->username, new_user->password, new_user->is_admin);
    fclose(file);
    return 1; // Registered successfully
}

int authenticate_user(const char *username, const char *password, User *user) {
    FILE *file = fopen("users.txt", "r");
    if (!file) {
        return 0; // No users registered
    }
    char line[256];
    while (fgets(line, sizeof(line), file)) {
        User u;
        if (sscanf(line, "%[^,],%[^,],%d", u.username, u.password, &u.is_admin) == 3) {
            if (strcmp(u.username, username) == 0 && strcmp(u.password, password) == 0) {
                if (user) {
                    *user = u;
                }
                fclose(file);
                return 1; // Authenticated
            }
        }
    }
    fclose(file);
    return 0; // Invalid credentials
}

// ============================================================
//  SECTION 5: ALGORITHMS — SORTING & SEARCHING
// ============================================================

// Bubble sort using pointer arithmetic to prepare database for binary search
void sort_questions_by_id(Question *questions, int count) {
    for (int i = 0; i < count - 1; i++) {
        for (int j = 0; j < count - i - 1; j++) {
            if ((questions + j)->id > (questions + j + 1)->id) {
                Question temp = *(questions + j);
                *(questions + j) = *(questions + j + 1);
                *(questions + j + 1) = temp;
            }
        }
    }
}

// Recursive Binary Search implementation
int recursive_binary_search(Question *questions, int low, int high, int target_id) {
    if (low > high) {
        return -1; // Base case: Target not found
    }
    int mid = low + (high - low) / 2;
    if (questions[mid].id == target_id) {
        return mid; // Base case: Target found
    } else if (questions[mid].id > target_id) {
        return recursive_binary_search(questions, low, mid - 1, target_id);
    } else {
        return recursive_binary_search(questions, mid + 1, high, target_id);
    }
}

// Recursive function to calculate average percentage, passing candidates, and top mark
void recursive_calculate_stats(ExamResult *results, int index, int count, float *sum_percentage, int *pass_count, float *max_percentage) {
    if (index == count) {
        return; // Base case: processed all records
    }
    
    *sum_percentage += results[index].percentage;
    if (results[index].percentage >= 50.0f) {
        (*pass_count)++;
    }
    if (results[index].percentage > *max_percentage) {
        *max_percentage = results[index].percentage;
    }
    
    // Recursive execution step
    recursive_calculate_stats(results, index + 1, count, sum_percentage, pass_count, max_percentage);
}

// ============================================================
//  SECTION 6: ADMIN PANEL ACTIONS
// ============================================================

void add_question() {
    print_banner("ADMIN: ADD NEW QUESTION");
    Question *questions = NULL;
    int count = load_questions(&questions);
    
    Question q;
    int new_id = 100; // Start ID numbering
    if (count > 0) {
        // Increment based on the highest existing ID to prevent duplication
        for (int i = 0; i < count; i++) {
            if (questions[i].id >= new_id) {
                new_id = questions[i].id + 1;
            }
        }
    }
    q.id = new_id;
    printf("Generating Question ID: %d\n\n", q.id);
    
    do {
        printf("Enter Question Description:\n> ");
        fgets(q.text, sizeof(q.text), stdin);
        remove_newline(q.text);
    } while (strlen(q.text) == 0);
    
    printf("\nChoose Question Type:\n  [0] Multiple Choice Question (MCQ)\n  [1] True / False Statement\nChoice: ");
    int type_sel;
    scanf("%d", &type_sel);
    q.type = (type_sel == 1) ? 1 : 0;
    
    if (q.type == 0) {
        printf("\nEnter Options (4 items needed):\n");
        for (int i = 0; i < 4; i++) {
            do {
                printf("  Option %c: ", 'A' + i);
                fgets(q.options[i], sizeof(q.options[i]), stdin);
                remove_newline(q.options[i]);
            } while (strlen(q.options[i]) == 0);
        }
        
        char answer_char;
        do {
            printf("\nSelect Correct Answer Option (A, B, C, D): ");
            scanf(" %c", &answer_char);
            if (answer_char >= 'a' && answer_char <= 'z') {
                answer_char = answer_char - 'a' + 'A';
            }
        } while (answer_char < 'A' || answer_char > 'D');
        
        q.correct_ans.mcq_choice = answer_char; // Union mapping
    } else {
        int tf_sel;
        do {
            printf("\nSelect Correct Answer (1 = True, 0 = False): ");
            scanf("%d", &tf_sel);
        } while (tf_sel != 0 && tf_sel != 1);
        
        q.correct_ans.tf_choice = tf_sel; // Union mapping
    }
    
    // Allocate space and append
    count++;
    Question *temp = (Question *)realloc(questions, count * sizeof(Question));
    if (!temp) {
        printf("Fatal: Out of memory during database expansion.\n");
        if (questions) free(questions);
        press_any_key();
        return;
    }
    questions = temp;
    questions[count - 1] = q;
    
    save_questions(questions, count);
    free(questions);
    
    printf("\n[SUCCESS] Question added to database successfully!\n");
    press_any_key();
}

void view_all_questions() {
    print_banner("ADMIN: EXAM QUESTIONS DATABASE");
    Question *questions = NULL;
    int count = load_questions(&questions);
    
    if (count == 0) {
        printf("No questions exist in database.\n");
        press_any_key();
        return;
    }
    
    // Sort array before viewing/binary search searchability
    sort_questions_by_id(questions, count);
    
    for (int i = 0; i < count; i++) {
        printf("ID: %-4d | Type: %s\n", questions[i].id, questions[i].type == 0 ? "MCQ" : "True/False");
        printf("Question: %s\n", questions[i].text);
        if (questions[i].type == 0) {
            for (int j = 0; j < 4; j++) {
                printf("  %c. %s\n", 'A' + j, questions[i].options[j]);
            }
            printf("Correct Ans: %c\n", questions[i].correct_ans.mcq_choice);
        } else {
            printf("Correct Ans: %s\n", questions[i].correct_ans.tf_choice ? "True" : "False");
        }
        printf("-----------------------------------------------------------------\n");
    }
    
    printf("\nDo you want to run binary search to query a specific ID? (1 = Yes, 0 = No): ");
    int s_opt;
    scanf("%d", &s_opt);
    if (s_opt == 1) {
        printf("Enter ID to search: ");
        int query_id;
        scanf("%d", &query_id);
        
        int idx = recursive_binary_search(questions, 0, count - 1, query_id);
        if (idx == -1) {
            printf("Question ID %d not found in records.\n", query_id);
        } else {
            printf("\n=== MATCH FOUND AT INDEX %d ===\n", idx);
            printf("ID: %-4d | Type: %s\n", questions[idx].id, questions[idx].type == 0 ? "MCQ" : "True/False");
            printf("Question: %s\n", questions[idx].text);
            if (questions[idx].type == 0) {
                for (int j = 0; j < 4; j++) {
                    printf("  %c. %s\n", 'A' + j, questions[idx].options[j]);
                }
                printf("Correct: %c\n", questions[idx].correct_ans.mcq_choice);
            } else {
                printf("Correct: %s\n", questions[idx].correct_ans.tf_choice ? "True" : "False");
            }
            printf("=================================\n");
        }
    }
    
    free(questions);
    press_any_key();
}

void delete_question() {
    print_banner("ADMIN: REMOVE QUESTION");
    Question *questions = NULL;
    int count = load_questions(&questions);
    
    if (count == 0) {
        printf("No questions exist to delete.\n");
        press_any_key();
        return;
    }
    
    sort_questions_by_id(questions, count);
    printf("Enter target Question ID to delete: ");
    int target_id;
    scanf("%d", &target_id);
    
    int index = recursive_binary_search(questions, 0, count - 1, target_id);
    if (index == -1) {
        printf("Question with ID %d could not be found.\n", target_id);
        free(questions);
        press_any_key();
        return;
    }
    
    // Shift elements left to overwrite target
    for (int i = index; i < count - 1; i++) {
        questions[i] = questions[i + 1];
    }
    count--;
    
    if (count > 0) {
        Question *temp = (Question *)realloc(questions, count * sizeof(Question));
        if (temp) {
            questions = temp;
        }
        save_questions(questions, count);
    } else {
        // database is now empty
        FILE *file = fopen("questions.txt", "w");
        if (file) fclose(file);
    }
    
    printf("[SUCCESS] Question with ID %d removed successfully.\n", target_id);
    free(questions);
    press_any_key();
}

void view_exam_statistics() {
    print_banner("ADMIN: CANDIDATE PERFORMANCE & STATS");
    ExamResult *results = NULL;
    int count = load_results(&results);
    
    if (count == 0) {
        printf("No candidate results recorded in results database yet.\n");
        press_any_key();
        return;
    }
    
    float total_percentage_sum = 0.0f;
    int pass_count = 0;
    float top_score = 0.0f;
    
    // Compute summary using recursion
    recursive_calculate_stats(results, 0, count, &total_percentage_sum, &pass_count, &top_score);
    
    float avg_percentage = total_percentage_sum / count;
    
    printf("\n*** EXAMINATION STATS DASHBOARD ***\n");
    printf("Completed Sessions  : %d\n", count);
    printf("Class Score Average : %.2f%%\n", avg_percentage);
    printf("Highest Score       : %.2f%%\n", top_score);
    printf("Passing Candidates  : %d\n", pass_count);
    printf("Class Passing Rate  : %.2f%%\n", ((float)pass_count / count) * 100.0f);
    printf("************************************\n");
    
    printf("\nDetailed Audit Logs:\n");
    printf("--------------------------------------------------------------------------------\n");
    printf("  %-25s | %-12s | %-15s | %-15s\n", "Candidate Name", "Raw Marks", "Out Of", "Percentage");
    printf("--------------------------------------------------------------------------------\n");
    for (int i = 0; i < count; i++) {
        printf("  %-25s | %-12d | %-15d | %-12.2f%% (%s)\n", 
               results[i].username, results[i].score, results[i].total_questions, results[i].percentage,
               results[i].percentage >= 50.0f ? "PASS" : "FAIL");
    }
    printf("--------------------------------------------------------------------------------\n");
    
    free(results);
    press_any_key();
}

// ============================================================
//  SECTION 7: STUDENT SESSION ACTIONS
// ============================================================

void take_exam(const User *student) {
    print_banner("STUDENT: ONLINE EXAM");
    Question *questions = NULL;
    int count = load_questions(&questions);
    
    if (count == 0) {
        printf("The exam is empty. There are no questions seeded by the administrator.\n");
        press_any_key();
        return;
    }
    
    printf("Welcome, %s. You are starting the exam.\n", student->username);
    printf("Total Questions: %d\n", count);
    printf("Instructions: MCQs have 4 choices (A, B, C, D). True/False have binary answers (1/0).\n");
    printf("Passing Limit is 50%%\n\n");
    
    show_loading("Configuring Exam Instance");
    
    int student_score = 0;
    
    for (int i = 0; i < count; i++) {
        clear_screen();
        print_banner("EXAMINATION IN PROGRESS");
        printf("Question %d of %d  |  ID: %d\n", i + 1, count, questions[i].id);
        printf("-----------------------------------------------------------------\n");
        printf("%s\n", questions[i].text);
        printf("-----------------------------------------------------------------\n");
        
        if (questions[i].type == 0) {
            // MCQ rendering
            for (int k = 0; k < 4; k++) {
                printf("   %c. %s\n", 'A' + k, questions[i].options[k]);
            }
            printf("\n");
            
            char answer_opt;
            do {
                printf("Your Answer Selection (A, B, C, D): ");
                scanf(" %c", &answer_opt);
                if (answer_opt >= 'a' && answer_opt <= 'z') {
                    answer_opt = answer_opt - 'a' + 'A';
                }
            } while (answer_opt < 'A' || answer_opt > 'D');
            
            if (answer_opt == questions[i].correct_ans.mcq_choice) {
                student_score++;
            }
        } else {
            // True/False rendering
            printf("   [1] True\n");
            printf("   [0] False\n\n");
            
            int answer_tf;
            do {
                printf("Your Answer Selection (1 = True, 0 = False): ");
                scanf("%d", &answer_tf);
            } while (answer_tf != 0 && answer_tf != 1);
            
            if (answer_tf == questions[i].correct_ans.tf_choice) {
                student_score++;
            }
        }
    }
    
    ExamResult result;
    strcpy(result.username, student->username);
    result.score = student_score;
    result.total_questions = count;
    result.percentage = ((float)student_score / count) * 100.0f;
    
    // Save student scorecard to results.txt file
    save_result(&result);
    
    clear_screen();
    print_banner("EXAM COMPLETED");
    show_loading("Submitting Responses & Generating Scorecard");
    
    printf("\nCongratulations, you have completed the exam session!\n\n");
    printf("\n*** SCORE CARD ***\n");
    printf("Candidate Name : %s\n", result.username);
    printf("Marks Scored   : %d / %d\n", result.score, result.total_questions);
    printf("Final Score    : %.2f%%\n", result.percentage);
    printf("Status Result  : %s\n", result.percentage >= 50.0f ? "PASS" : "FAIL");
    printf("******************\n\n");
    
    free(questions);
    press_any_key();
}

void view_my_results(const User *student) {
    print_banner("STUDENT: YOUR HISTORY");
    ExamResult *results = NULL;
    int count = load_results(&results);
    
    if (count == 0) {
        printf("You have no previous examinations logged.\n");
        press_any_key();
        return;
    }
    
    int records_found = 0;
    printf("Showing records matching username: %s\n", student->username);
    printf("---------------------------------------------------------------------------------\n");
    printf("  %-10s | %-12s | %-15s | %-15s\n", "Attempt", "Marks Obtained", "Total Questions", "Percentage");
    printf("---------------------------------------------------------------------------------\n");
    
    int attempt_counter = 1;
    for (int i = 0; i < count; i++) {
        if (strcmp(results[i].username, student->username) == 0) {
            printf("  Attempt %-2d | %-14d | %-15d | %-12.2f%% (%s)\n", 
                   attempt_counter++, results[i].score, results[i].total_questions, results[i].percentage,
                   results[i].percentage >= 50.0f ? "PASS" : "FAIL");
            records_found = 1;
        }
    }
    printf("---------------------------------------------------------------------------------\n");
    
    if (!records_found) {
        printf("No attempts logged for user %s.\n", student->username);
    }
    
    free(results);
    press_any_key();
}

// ============================================================
//  SECTION 8: DATABASE SEEDING
// ============================================================

// Automatically populate questions and users text databases if empty
void seed_initial_data() {
    // 1. Seed admin and student credentials if users.txt is empty
    FILE *file = fopen("users.txt", "r");
    int empty = 1;
    if (file) {
        char line[256];
        if (fgets(line, sizeof(line), file)) {
            empty = 0;
        }
        fclose(file);
    }
    
    if (empty) {
        User admin;
        strcpy(admin.username, "admin");
        strcpy(admin.password, "admin123");
        admin.is_admin = 1;
        register_user(&admin);
        
        User student;
        strcpy(student.username, "student");
        strcpy(student.password, "student123");
        student.is_admin = 0;
        register_user(&student);
    }
    
    // 2. Seed diagnostic questions if questions.txt is empty
    file = fopen("questions.txt", "r");
    empty = 1;
    if (file) {
        char line[256];
        if (fgets(line, sizeof(line), file)) {
            empty = 0;
        }
        fclose(file);
    }
    
    if (empty) {
        Question q[4];
        
        // Q1 - MCQ
        q[0].id = 101;
        strcpy(q[0].text, "Which C keyword is used to allocate memory dynamically?");
        q[0].type = 0;
        strcpy(q[0].options[0], "alloc");
        strcpy(q[0].options[1], "malloc");
        strcpy(q[0].options[2], "new");
        strcpy(q[0].options[3], "create");
        q[0].correct_ans.mcq_choice = 'B';
        
        // Q2 - MCQ
        q[1].id = 102;
        strcpy(q[1].text, "Which data structure member-referencing operator is used with structure pointers?");
        q[1].type = 0;
        strcpy(q[1].options[0], "dot operator (.)");
        strcpy(q[1].options[1], "arrow operator (->)");
        strcpy(q[1].options[2], "asterisk (*)");
        strcpy(q[1].options[3], "ampersand (&)");
        q[1].correct_ans.mcq_choice = 'B';
        
        // Q3 - True/False
        q[2].id = 103;
        strcpy(q[2].text, "In a C union, all members share the same memory location.");
        q[2].type = 1;
        q[2].correct_ans.tf_choice = 1; // True
        
        // Q4 - True/False
        q[3].id = 104;
        strcpy(q[3].text, "Standard C arrays perform automatic boundary checks during compile and execution.");
        q[3].type = 1;
        q[3].correct_ans.tf_choice = 0; // False
        
        save_questions(q, 4);
    }
}

// ============================================================
//  SECTION 9: DASHBOARD MENUS
// ============================================================

// Submenu for administrator operations
void handle_admin_dashboard(User *admin) {
    int choice;
    do {
        print_banner("ADMINISTRATOR MAIN MENU");
        printf("Logged in as: %s (Admin)\n\n", admin->username);
        printf("  [1] Add Exam Question\n");
        printf("  [2] View & Search Questions\n");
        printf("  [3] Delete Exam Question\n");
        printf("  [4] View Exam Statistics & Logs\n");
        printf("  [5] Log Out\n\n");
        printf("Select Action: ");
        if (scanf("%d", &choice) != 1) {
            choice = 0;
            // Clear standard input error flag and buffer
            int c;
            while ((c = getchar()) != '\n' && c != EOF);
        }
        
        switch (choice) {
            case 1:
                add_question();
                break;
            case 2:
                view_all_questions();
                break;
            case 3:
                delete_question();
                break;
            case 4:
                view_exam_statistics();
                break;
            case 5:
                show_loading("Logging out administrative session");
                break;
            default:
                printf("Invalid selection! Try again.\n");
                press_any_key();
        }
    } while (choice != 5);
}

// Submenu for student/candidate operations
void handle_student_dashboard(User *student) {
    int choice;
    do {
        print_banner("STUDENT MAIN MENU");
        printf("Logged in as: %s\n\n", student->username);
        printf("  [1] Start Examination\n");
        printf("  [2] View Your Scorecard History\n");
        printf("  [3] Log Out\n\n");
        printf("Select Action: ");
        if (scanf("%d", &choice) != 1) {
            choice = 0;
            int c;
            while ((c = getchar()) != '\n' && c != EOF);
        }
        
        switch (choice) {
            case 1:
                take_exam(student);
                break;
            case 2:
                view_my_results(student);
                break;
            case 3:
                show_loading("Logging out student session");
                break;
            default:
                printf("Invalid selection! Try again.\n");
                press_any_key();
        }
    } while (choice != 3);
}

// ============================================================
//  SECTION 10: MAIN ENTRY POINT
// ============================================================

int main() {
    // Automatically initialize databases on startup
    seed_initial_data();
    
    char username[MAX_NAME_LEN];
    char password[MAX_NAME_LEN];
    User user;

    int choice;
    do {
        print_banner("WELCOME TO THE ONLINE EXAM SYSTEM");
        printf("  [1] Student Login\n");
        printf("  [2] Student Registration\n");
        printf("  [3] Administrator Login\n");
        printf("  [4] Exit Application\n\n");
        printf("Select Action: ");
        if (scanf("%d", &choice) != 1) {
            choice = 0;
            int c;
            while ((c = getchar()) != '\n' && c != EOF);
        }

        switch (choice) {
            case 1: // Student Login
                print_banner("STUDENT LOGIN");
                printf("Enter Username: ");
                scanf("%49s", username);
                printf("Enter Password: ");
                get_masked_password(password, MAX_NAME_LEN);
                
                show_loading("Authenticating Candidate credentials");
                if (authenticate_user(username, password, &user)) {
                    if (user.is_admin == 0) {
                        handle_student_dashboard(&user);
                    } else {
                        printf("\nAccess Denied: This account is flagged as Administrator.\n");
                        press_any_key();
                    }
                } else {
                    printf("\nError: Invalid Username or Password.\n");
                    press_any_key();
                }
                break;
                
            case 2: // Student Registration
                print_banner("NEW CANDIDATE REGISTRATION");
                User new_student;
                printf("Create Username: ");
                scanf("%49s", new_student.username);
                printf("Create Password: ");
                scanf("%49s", new_student.password);
                new_student.is_admin = 0; // Students are non-admin accounts
                
                show_loading("Registering new candidate profile");
                int reg_status = register_user(&new_student);
                if (reg_status == 1) {
                    printf("\n[SUCCESS] Registration completed! You can now log in.\n");
                } else if (reg_status == 0) {
                    printf("\nError: Username already exists! Please choose another.\n");
                } else {
                    printf("\nError: Database access failed.\n");
                }
                press_any_key();
                break;
                
            case 3: // Admin Login
                print_banner("ADMINISTRATOR SECURITY PORTAL");
                printf("Enter Admin Username: ");
                scanf("%49s", username);
                printf("Enter Admin Password: ");
                get_masked_password(password, MAX_NAME_LEN);
                
                show_loading("Verifying Administrative security credentials");
                if (authenticate_user(username, password, &user)) {
                    if (user.is_admin == 1) {
                        handle_admin_dashboard(&user);
                    } else {
                        printf("\nAccess Denied: Account does not possess Administrative privileges.\n");
                        press_any_key();
                    }
                } else {
                    printf("\nError: Invalid Admin Credentials.\n");
                    press_any_key();
                }
                break;
                
            case 4:
                print_banner("EXITING SYSTEM");
                printf("Thank you for using the Online Examination System.\n");
                printf("System shutting down cleanly...\n");
                break;
                
            default:
                printf("Invalid Option. Please choose between 1 and 4.\n");
                press_any_key();
        }
    } while (choice != 4);
    
    return 0;
}

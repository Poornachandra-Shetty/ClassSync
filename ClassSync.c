#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define MAX_LINE 1024
#define MAX_NAME 100
#define MAX_SECTIONS 10
#define MAX_DAYS 6
#define MAX_PERIODS 8
#define MAX_FACULTY 50
#define MAX_SUBJECTS 100

// Structure Definitions
typedef struct {
    int id;
    char name[MAX_NAME];
    int maxHours;
    int assignedHours;
} Faculty;

typedef struct {
    int id;
    char name[MAX_NAME];
    int facultyId;
    int hoursPerWeek;
    char sections[MAX_SECTIONS][10];
    int sectionCount;
} Subject;

typedef struct {
    char branch[MAX_NAME];
    char sections[MAX_SECTIONS][10];
    int sectionCount;
} Branch;

typedef struct {
    int day;
    int periods;
} DaySlot;

typedef struct {
    int facultyId;
    int subjectId;
    char section[10];
} TimeSlot;

// Queue Node for scheduling algorithm
typedef struct QueueNode {
    int subjectId;
    char section[10];
    int remainingHours;
    struct QueueNode* next;
} QueueNode;

typedef struct {
    QueueNode* front;
    QueueNode* rear;
} Queue;

// Global Data
Faculty faculties[MAX_FACULTY];
Subject subjects[MAX_SUBJECTS];
Branch branch;
DaySlot days[MAX_DAYS];
TimeSlot timetable[MAX_DAYS][MAX_PERIODS][MAX_SECTIONS];
int facultyCount = 0, subjectCount = 0, dayCount = 0;

// Queue Operations
Queue* createQueue() {
    Queue* q = (Queue*)malloc(sizeof(Queue));
    q->front = q->rear = NULL;
    return q;
}

void enqueue(Queue* q, int subId, char* sec, int hrs) {
    QueueNode* node = (QueueNode*)malloc(sizeof(QueueNode));
    node->subjectId = subId;
    strcpy(node->section, sec);
    node->remainingHours = hrs;
    node->next = NULL;
    
    if (q->rear == NULL) {
        q->front = q->rear = node;
    } else {
        q->rear->next = node;
        q->rear = node;
    }
}

QueueNode* dequeue(Queue* q) {
    if (q->front == NULL) return NULL;
    QueueNode* temp = q->front;
    q->front = q->front->next;
    if (q->front == NULL) q->rear = NULL;
    return temp;
}

bool isQueueEmpty(Queue* q) {
    return q->front == NULL;
}

// Utility Functions
void trim(char* str) {
    char* start = str;
    char* end;
    
    // Trim leading space
    while (*start == ' ' || *start == '\t' || *start == '\n' || *start == '\r') start++;
    
    // All spaces?
    if (*start == 0) {
        *str = 0;
        return;
    }
    
    // Trim trailing space
    end = start + strlen(start) - 1;
    while (end > start && (*end == ' ' || *end == '\t' || *end == '\n' || *end == '\r')) end--;
    
    // Write new null terminator
    *(end + 1) = 0;
    
    // Move trimmed string to beginning
    if (start != str) {
        memmove(str, start, strlen(start) + 1);
    }
}

int getSectionIndex(char* section) {
    for (int i = 0; i < branch.sectionCount; i++) {
        if (strcmp(branch.sections[i], section) == 0) return i;
    }
    return -1;
}

// CSV Reading Functions
void readFacultyCSV(const char* filename) {
    FILE* fp = fopen(filename, "r");
    if (!fp) {
        printf("Error: Cannot open %s\n", filename);
        return;
    }
    
    char line[MAX_LINE];
    fgets(line, MAX_LINE, fp); // Skip header
    
    while (fgets(line, MAX_LINE, fp)) {
        char* token = strtok(line, ",");
        faculties[facultyCount].id = atoi(token);
        
        token = strtok(NULL, ",");
        strcpy(faculties[facultyCount].name, token);
        trim(faculties[facultyCount].name);
        
        token = strtok(NULL, ",");
        faculties[facultyCount].maxHours = atoi(token);
        faculties[facultyCount].assignedHours = 0;
        
        facultyCount++;
    }
    fclose(fp);
    printf("Loaded %d faculties\n", facultyCount);
}

void readSubjectsCSV(const char* filename) {
    FILE* fp = fopen(filename, "r");
    if (!fp) {
        printf("Error: Cannot open %s\n", filename);
        return;
    }
    
    char line[MAX_LINE];
    fgets(line, MAX_LINE, fp); // Skip header
    
    while (fgets(line, MAX_LINE, fp)) {
        // Remove newline
        line[strcspn(line, "\r\n")] = 0;
        
        char* token = strtok(line, ",");
        subjects[subjectCount].id = atoi(token);
        
        token = strtok(NULL, ",");
        strcpy(subjects[subjectCount].name, token);
        trim(subjects[subjectCount].name);
        
        token = strtok(NULL, ",");
        subjects[subjectCount].facultyId = atoi(token);
        
        token = strtok(NULL, ",");
        subjects[subjectCount].hoursPerWeek = atoi(token);
        
        token = strtok(NULL, ",");
        if (token != NULL) {
            trim(token);
            char sectionsBuffer[MAX_LINE];
            strcpy(sectionsBuffer, token);
            
            char* secToken = strtok(sectionsBuffer, ";");
            subjects[subjectCount].sectionCount = 0;
            while (secToken != NULL && subjects[subjectCount].sectionCount < MAX_SECTIONS) {
                trim(secToken);
                strcpy(subjects[subjectCount].sections[subjects[subjectCount].sectionCount], secToken);
                subjects[subjectCount].sectionCount++;
                secToken = strtok(NULL, ";");
            }
        }
        
        subjectCount++;
    }
    fclose(fp);
    printf("Loaded %d subjects\n", subjectCount);
}

void readSectionsCSV(const char* filename) {
    FILE* fp = fopen(filename, "r");
    if (!fp) {
        printf("Error: Cannot open %s\n", filename);
        return;
    }
    
    char line[MAX_LINE];
    fgets(line, MAX_LINE, fp); // Skip header
    
    if (fgets(line, MAX_LINE, fp)) {
        // Remove newline
        line[strcspn(line, "\r\n")] = 0;
        
        char* token = strtok(line, ",");
        strcpy(branch.branch, token);
        trim(branch.branch);
        
        token = strtok(NULL, ",");
        if (token != NULL) {
            trim(token);
            char* secToken = strtok(token, ";");
            branch.sectionCount = 0;
            while (secToken != NULL && branch.sectionCount < MAX_SECTIONS) {
                trim(secToken);
                strcpy(branch.sections[branch.sectionCount], secToken);
                printf("  Section loaded: '%s'\n", branch.sections[branch.sectionCount]);
                branch.sectionCount++;
                secToken = strtok(NULL, ";");
            }
        }
    }
    fclose(fp);
    printf("Loaded branch: %s with %d sections\n", branch.branch, branch.sectionCount);
}

void readSlotsCSV(const char* filename) {
    FILE* fp = fopen(filename, "r");
    if (!fp) {
        printf("Error: Cannot open %s\n", filename);
        return;
    }
    
    char line[MAX_LINE];
    fgets(line, MAX_LINE, fp); // Skip header
    
    while (fgets(line, MAX_LINE, fp)) {
        char* token = strtok(line, ",");
        days[dayCount].day = atoi(token);
        
        token = strtok(NULL, ",");
        days[dayCount].periods = atoi(token);
        
        dayCount++;
    }
    fclose(fp);
    printf("Loaded %d days\n", dayCount);
}

// Conflict Detection
bool isFacultyFree(int facultyId, int day, int period) {
    for (int s = 0; s < branch.sectionCount; s++) {
        if (timetable[day][period][s].facultyId == facultyId) {
            return false;
        }
    }
    return true;
}

bool canAssign(int facultyId, int day, int period, int sectionIdx) {
    // Check if slot is empty
    if (timetable[day][period][sectionIdx].facultyId != -1) return false;
    
    // Check if faculty is free
    if (!isFacultyFree(facultyId, day, period)) return false;
    
    // Check faculty workload
    for (int f = 0; f < facultyCount; f++) {
        if (faculties[f].id == facultyId) {
            if (faculties[f].assignedHours >= faculties[f].maxHours) return false;
            break;
        }
    }
    
    return true;
}

// Timetable Generation using Improved Queue-based Greedy Algorithm
void generateTimetable() {
    // Initialize timetable with -1 (empty slots)
    for (int d = 0; d < dayCount; d++) {
        for (int p = 0; p < days[d].periods; p++) {
            for (int s = 0; s < branch.sectionCount; s++) {
                timetable[d][p][s].facultyId = -1;
                timetable[d][p][s].subjectId = -1;
                strcpy(timetable[d][p][s].section, "");
            }
        }
    }
    
    // Create queue and enqueue all subject-section combinations
    Queue* q = createQueue();
    for (int i = 0; i < subjectCount; i++) {
        for (int j = 0; j < subjects[i].sectionCount; j++) {
            enqueue(q, subjects[i].id, subjects[i].sections[j], subjects[i].hoursPerWeek);
            printf("Enqueued: Subject %d for Section %s, Hours: %d\n", 
                   subjects[i].id, subjects[i].sections[j], subjects[i].hoursPerWeek);
        }
    }
    
    int totalAssigned = 0;
    int currentDay = 0;
    int maxAttemptsPerItem = 100; // Prevent infinite loops
    
    // Improved scheduling: distribute across all days
    while (!isQueueEmpty(q)) {
        QueueNode* node = dequeue(q);
        if (node == NULL) break;
        
        bool assigned = false;
        int sectionIdx = getSectionIndex(node->section);
        
        if (sectionIdx == -1) {
            printf("Warning: Section %s not found!\n", node->section);
            free(node);
            continue;
        }
        
        // Find subject and faculty
        int subIdx = -1, facIdx = -1;
        for (int i = 0; i < subjectCount; i++) {
            if (subjects[i].id == node->subjectId) {
                subIdx = i;
                break;
            }
        }
        
        if (subIdx == -1) {
            printf("Warning: Subject %d not found!\n", node->subjectId);
            free(node);
            continue;
        }
        
        for (int i = 0; i < facultyCount; i++) {
            if (faculties[i].id == subjects[subIdx].facultyId) {
                facIdx = i;
                break;
            }
        }
        
        if (facIdx == -1) {
            printf("Warning: Faculty %d not found!\n", subjects[subIdx].facultyId);
            free(node);
            continue;
        }
        
        // Try to assign starting from current day (round-robin distribution)
        int attempts = 0;
        for (int d = 0; d < dayCount && !assigned; d++) {
            int day = (currentDay + d) % dayCount;
            for (int p = 0; p < days[day].periods && !assigned; p++) {
                if (canAssign(subjects[subIdx].facultyId, day, p, sectionIdx)) {
                    timetable[day][p][sectionIdx].facultyId = subjects[subIdx].facultyId;
                    timetable[day][p][sectionIdx].subjectId = subjects[subIdx].id;
                    strcpy(timetable[day][p][sectionIdx].section, node->section);
                    
                    faculties[facIdx].assignedHours++;
                    totalAssigned++;
                    
                    printf("Assigned: %s to Section %s, Day %d, Period %d (Faculty: %s)\n",
                           subjects[subIdx].name, node->section, day+1, p+1, faculties[facIdx].name);
                    
                    node->remainingHours--;
                    if (node->remainingHours > 0) {
                        enqueue(q, node->subjectId, node->section, node->remainingHours);
                    }
                    assigned = true;
                    currentDay = (day + 1) % dayCount; // Move to next day for distribution
                }
            }
        }
        
        if (!assigned) {
            printf("Could not assign: Subject %d (%s) to Section %s (remaining hours: %d)\n",
                   node->subjectId, subjects[subIdx].name, node->section, node->remainingHours);
            
            // Try to re-enqueue with lower priority if we haven't tried too many times
            if (attempts < maxAttemptsPerItem && node->remainingHours > 0) {
                enqueue(q, node->subjectId, node->section, node->remainingHours);
            }
        }
        
        free(node);
    }
    
    free(q);
    printf("\nTimetable generated successfully!\n");
    printf("Total classes assigned: %d\n", totalAssigned);
    
    // Print day-wise distribution
    printf("\nDay-wise class distribution:\n");
    for (int d = 0; d < dayCount; d++) {
        int dayTotal = 0;
        for (int p = 0; p < days[d].periods; p++) {
            for (int s = 0; s < branch.sectionCount; s++) {
                if (timetable[d][p][s].facultyId != -1) {
                    dayTotal++;
                }
            }
        }
        printf("Day %d: %d classes\n", d+1, dayTotal);
    }
}

// Output Generation
void generateSectionTimetable() {
    FILE* fp = fopen("section_timetable.csv", "w");
    
    // Write header
    fprintf(fp, "Section,Day,Period,Subject,Faculty\n");
    
    // Sort by section, then day, then period for organized output
    for (int s = 0; s < branch.sectionCount; s++) {
        for (int d = 0; d < dayCount; d++) {
            for (int p = 0; p < days[d].periods; p++) {
                if (timetable[d][p][s].facultyId != -1) {
                    char subName[MAX_NAME] = "Unknown";
                    char facName[MAX_NAME] = "Unknown";
                    
                    // Find subject name
                    for (int i = 0; i < subjectCount; i++) {
                        if (subjects[i].id == timetable[d][p][s].subjectId) {
                            strcpy(subName, subjects[i].name);
                            break;
                        }
                    }
                    
                    // Find faculty name
                    for (int i = 0; i < facultyCount; i++) {
                        if (faculties[i].id == timetable[d][p][s].facultyId) {
                            strcpy(facName, faculties[i].name);
                            break;
                        }
                    }
                    
                    // Write CSV line with proper formatting
                    fprintf(fp, "\"%s\",%d,%d,\"%s\",\"%s\"\n", 
                            branch.sections[s], d+1, p+1, subName, facName);
                }
            }
        }
    }
    
    fclose(fp);
    printf("Generated section_timetable.csv\n");
}

void generateFacultyTimetable() {
    FILE* fp = fopen("faculty_timetable.csv", "w");
    
    // Write header
    fprintf(fp, "Faculty,Day,Period,Subject,Section\n");
    
    // Sort by faculty, then day, then period
    for (int f = 0; f < facultyCount; f++) {
        for (int d = 0; d < dayCount; d++) {
            for (int p = 0; p < days[d].periods; p++) {
                for (int s = 0; s < branch.sectionCount; s++) {
                    if (timetable[d][p][s].facultyId == faculties[f].id) {
                        char subName[MAX_NAME] = "Unknown";
                        
                        // Find subject name
                        for (int i = 0; i < subjectCount; i++) {
                            if (subjects[i].id == timetable[d][p][s].subjectId) {
                                strcpy(subName, subjects[i].name);
                                break;
                            }
                        }
                        
                        // Write CSV line with proper formatting
                        fprintf(fp, "\"%s\",%d,%d,\"%s\",\"%s\"\n", 
                                faculties[f].name, d+1, p+1, subName, branch.sections[s]);
                    }
                }
            }
        }
    }
    
    fclose(fp);
    printf("Generated faculty_timetable.csv\n");
}

void generateSummary() {
    FILE* fp = fopen("summary.csv", "w");
    fprintf(fp, "FacultyID,FacultyName,MaxHours,AssignedHours,Utilization%%\n");
    
    for (int i = 0; i < facultyCount; i++) {
        float util = (faculties[i].maxHours > 0) ? 
                     (faculties[i].assignedHours * 100.0 / faculties[i].maxHours) : 0;
        fprintf(fp, "%d,%s,%d,%d,%.2f\n", 
                faculties[i].id, faculties[i].name, 
                faculties[i].maxHours, faculties[i].assignedHours, util);
    }
    fclose(fp);
    printf("Generated summary.csv\n");
}

// Main Function
int main() {
    printf("=== ClassSync: Faculty Timetable Generator ===\n\n");
    
    // Read input CSV files
    readFacultyCSV("faculty.csv");
    readSubjectsCSV("subjects.csv");
    readSectionsCSV("sections.csv");
    readSlotsCSV("slots.csv");
    
    // Calculate and display workload statistics
    printf("\n=== Workload Analysis ===\n");
    int totalRequired = 0;
    for (int i = 0; i < subjectCount; i++) {
        int subjectTotal = subjects[i].hoursPerWeek * subjects[i].sectionCount;
        totalRequired += subjectTotal;
        printf("Subject %d: %d hours/week × %d sections = %d total hours\n",
               subjects[i].id, subjects[i].hoursPerWeek, 
               subjects[i].sectionCount, subjectTotal);
    }
    
    int totalCapacity = 0;
    for (int i = 0; i < facultyCount; i++) {
        totalCapacity += faculties[i].maxHours;
    }
    
    int totalSlots = 0;
    for (int i = 0; i < dayCount; i++) {
        totalSlots += days[i].periods;
    }
    totalSlots *= branch.sectionCount;
    
    printf("\nTotal classes required: %d\n", totalRequired);
    printf("Total faculty capacity: %d hours\n", totalCapacity);
    printf("Total available slots: %d (Days × Periods × Sections)\n", totalSlots);
    
    if (totalRequired > totalCapacity) {
        printf("\n⚠️  WARNING: Required classes (%d) exceed faculty capacity (%d)!\n", 
               totalRequired, totalCapacity);
        printf("   Recommendation: Increase MaxHoursPerWeek for faculties or add more faculty.\n");
    }
    
    if (totalRequired > totalSlots) {
        printf("\n⚠️  WARNING: Required classes (%d) exceed available slots (%d)!\n", 
               totalRequired, totalSlots);
        printf("   Recommendation: Add more days or increase periods per day.\n");
    }
    
    printf("\n=== Generating Timetable ===\n");
    generateTimetable();
    
    printf("\n=== Generating Output Files ===\n");
    generateSectionTimetable();
    generateFacultyTimetable();
    generateSummary();
    
    printf("\n=== Timetable Generation Complete! ===\n");
    printf("Output files created:\n");
    printf("  - section_timetable.csv\n");
    printf("  - faculty_timetable.csv\n");
    printf("  - summary.csv\n");
    
    return 0;
}
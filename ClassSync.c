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

// Global Data
Faculty faculties[MAX_FACULTY];
Subject subjects[MAX_SUBJECTS];
Branch branch;
DaySlot days[MAX_DAYS];
TimeSlot timetable[MAX_DAYS][MAX_PERIODS][MAX_SECTIONS];
int facultyCount = 0, subjectCount = 0, dayCount = 0;

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

// Forward declarations for constraint checking functions
bool isFacultyFree(int facultyId, int day, int period);
bool hasConsecutiveClass(int subjectId, int sectionIdx, int day, int period);
int countSubjectInDay(int subjectId, int sectionIdx, int day);
bool canAssign(int facultyId, int day, int period, int sectionIdx);
bool canAssignWithConstraints(int subjectId, int facultyId, int day, int period, int sectionIdx);

// Conflict Detection Implementation
bool isFacultyFree(int facultyId, int day, int period) {
    for (int s = 0; s < branch.sectionCount; s++) {
        if (timetable[day][period][s].facultyId == facultyId) {
            return false;
        }
    }
    return true;
}

bool hasConsecutiveClass(int subjectId, int sectionIdx, int day, int period) {
    // Check previous period
    if (period > 0) {
        if (timetable[day][period-1][sectionIdx].subjectId == subjectId) {
            return true;
        }
    }
    
    // Check next period
    if (period < days[day].periods - 1) {
        if (timetable[day][period+1][sectionIdx].subjectId == subjectId) {
            return true;
        }
    }
    
    return false;
}

int countSubjectInDay(int subjectId, int sectionIdx, int day) {
    int count = 0;
    for (int p = 0; p < days[day].periods; p++) {
        if (timetable[day][p][sectionIdx].subjectId == subjectId) {
            count++;
        }
    }
    return count;
}

bool canAssign(int facultyId, int day, int period, int sectionIdx) {
    // Check if slot is empty
    if (timetable[day][period][sectionIdx].facultyId != -1) return false;
    
    // Check if faculty is free at this time
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

bool canAssignWithConstraints(int subjectId, int facultyId, int day, int period, int sectionIdx) {
    // Basic checks
    if (!canAssign(facultyId, day, period, sectionIdx)) return false;
    
    // Constraint 1: No consecutive classes of same subject
    if (hasConsecutiveClass(subjectId, sectionIdx, day, period)) return false;
    
    // Constraint 2: Maximum 2 classes of same subject per day
    if (countSubjectInDay(subjectId, sectionIdx, day) >= 2) return false;
    
    return true;
}

int countClassesInDay(int day, int sectionIdx) {
    int count = 0;
    for (int p = 0; p < days[day].periods; p++) {
        if (timetable[day][p][sectionIdx].facultyId != -1) {
            count++;
        }
    }
    return count;
}

bool findAndAssignSlot(int subjectId, char* section, int* assignedDay, int* assignedPeriod) {
    int sectionIdx = getSectionIndex(section);
    if (sectionIdx == -1) return false;
    
    int subIdx = -1, facIdx = -1;
    for (int i = 0; i < subjectCount; i++) {
        if (subjects[i].id == subjectId) {
            subIdx = i;
            break;
        }
    }
    if (subIdx == -1) return false;
    
    for (int i = 0; i < facultyCount; i++) {
        if (faculties[i].id == subjects[subIdx].facultyId) {
            facIdx = i;
            break;
        }
    }
    if (facIdx == -1) return false;
    
    // Strategy: Prefer days with fewer classes to distribute evenly
    typedef struct {
        int day;
        int period;
        int dayLoad;
    } SlotOption;
    
    SlotOption options[MAX_DAYS * MAX_PERIODS];
    int optionCount = 0;
    
    // Collect all valid slot options with their day loads
    for (int d = 0; d < dayCount; d++) {
        int dayLoad = countClassesInDay(d, sectionIdx);
        for (int p = 0; p < days[d].periods; p++) {
            if (canAssignWithConstraints(subjects[subIdx].id, subjects[subIdx].facultyId, d, p, sectionIdx)) {
                options[optionCount].day = d;
                options[optionCount].period = p;
                options[optionCount].dayLoad = dayLoad;
                optionCount++;
            }
        }
    }
    
    if (optionCount == 0) return false;
    
    // Sort options by day load (prefer days with fewer classes)
    for (int i = 0; i < optionCount - 1; i++) {
        for (int j = 0; j < optionCount - i - 1; j++) {
            if (options[j].dayLoad > options[j+1].dayLoad) {
                SlotOption temp = options[j];
                options[j] = options[j+1];
                options[j+1] = temp;
            }
        }
    }
    
    // Choose the first option (day with least load)
    int bestDay = options[0].day;
    int bestPeriod = options[0].period;
    
    // Assign to this slot
    timetable[bestDay][bestPeriod][sectionIdx].facultyId = subjects[subIdx].facultyId;
    timetable[bestDay][bestPeriod][sectionIdx].subjectId = subjects[subIdx].id;
    strcpy(timetable[bestDay][bestPeriod][sectionIdx].section, section);
    
    faculties[facIdx].assignedHours++;
    *assignedDay = bestDay;
    *assignedPeriod = bestPeriod;
    return true;
}

// Timetable Generation
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
    
    // Build a list of all assignments needed
    typedef struct {
        int subjectId;
        char section[10];
        char subjectName[MAX_NAME];
        int facultyId;
    } Assignment;
    
    Assignment assignments[500];
    int assignmentCount = 0;
    int totalToAssign = 0;
    
    for (int i = 0; i < subjectCount; i++) {
        for (int j = 0; j < subjects[i].sectionCount; j++) {
            for (int h = 0; h < subjects[i].hoursPerWeek; h++) {
                assignments[assignmentCount].subjectId = subjects[i].id;
                strcpy(assignments[assignmentCount].section, subjects[i].sections[j]);
                strcpy(assignments[assignmentCount].subjectName, subjects[i].name);
                assignments[assignmentCount].facultyId = subjects[i].facultyId;
                assignmentCount++;
                totalToAssign++;
            }
        }
    }
    
    printf("\nTotal classes to assign: %d\n", totalToAssign);
    printf("Starting assignment process...\n\n");
    
    int totalAssigned = 0;
    
    // Try to assign each class
    for (int i = 0; i < assignmentCount; i++) {
        int assignedDay = -1, assignedPeriod = -1;
        
        if (findAndAssignSlot(assignments[i].subjectId, assignments[i].section, 
                              &assignedDay, &assignedPeriod)) {
            totalAssigned++;
            
            // Find faculty name for display
            char facName[MAX_NAME] = "Unknown";
            for (int f = 0; f < facultyCount; f++) {
                if (faculties[f].id == assignments[i].facultyId) {
                    strcpy(facName, faculties[f].name);
                    break;
                }
            }
            
            if (totalAssigned <= 20 || totalAssigned % 10 == 0) {
                printf("✓ [%d/%d] Assigned: %s to Section %s, Day %d, Period %d (Faculty: %s)\n",
                       totalAssigned, totalToAssign, assignments[i].subjectName, 
                       assignments[i].section, assignedDay+1, assignedPeriod+1, facName);
            }
        } else {
            printf("✗ Failed to assign: %s to Section %s (Faculty ID: %d)\n",
                   assignments[i].subjectName, assignments[i].section, assignments[i].facultyId);
        }
    }
    
    printf("\n=== Timetable Generation Summary ===\n");
    printf("Total classes assigned: %d out of %d required (%.1f%%)\n", 
           totalAssigned, totalToAssign, (totalAssigned * 100.0 / totalToAssign));
    
    // Print day-wise distribution
    printf("\n=== Day-wise Class Distribution ===\n");
    for (int d = 0; d < dayCount; d++) {
        int dayTotal = 0;
        printf("Day %d: ", d+1);
        for (int s = 0; s < branch.sectionCount; s++) {
            int sectionClasses = 0;
            for (int p = 0; p < days[d].periods; p++) {
                if (timetable[d][p][s].facultyId != -1) {
                    sectionClasses++;
                    dayTotal++;
                }
            }
            printf("Sec-%s:%d  ", branch.sections[s], sectionClasses);
        }
        printf(" | Total: %d classes\n", dayTotal);
    }
    
    // Verify constraints
    printf("\n=== Constraint Verification ===\n");
    int consecutiveViolations = 0;
    int maxPerDayViolations = 0;
    
    for (int s = 0; s < branch.sectionCount; s++) {
        for (int d = 0; d < dayCount; d++) {
            // Check for consecutive classes
            for (int p = 0; p < days[d].periods - 1; p++) {
                if (timetable[d][p][s].subjectId != -1 && 
                    timetable[d][p][s].subjectId == timetable[d][p+1][s].subjectId) {
                    consecutiveViolations++;
                    printf("⚠️  Consecutive: Section %s, Day %d, Periods %d-%d\n", 
                           branch.sections[s], d+1, p+1, p+2);
                }
            }
            
            // Check max 2 per day constraint
            int subjectCounts[MAX_SUBJECTS] = {0};
            for (int p = 0; p < days[d].periods; p++) {
                if (timetable[d][p][s].subjectId != -1) {
                    for (int i = 0; i < subjectCount; i++) {
                        if (subjects[i].id == timetable[d][p][s].subjectId) {
                            subjectCounts[i]++;
                            if (subjectCounts[i] > 2) {
                                maxPerDayViolations++;
                                printf("⚠️  >2 classes/day: Section %s, Day %d, Subject %s (%d classes)\n", 
                                       branch.sections[s], d+1, subjects[i].name, subjectCounts[i]);
                            }
                            break;
                        }
                    }
                }
            }
        }
    }
    
    if (consecutiveViolations == 0 && maxPerDayViolations == 0) {
        printf("✓ All constraints satisfied!\n");
        printf("  - No consecutive classes of same subject\n");
        printf("  - Maximum 2 classes per subject per day\n");
        printf("  - Classes distributed across all %d days\n", dayCount);
    }
    
    // Print faculty workload
    printf("\n=== Faculty Workload ===\n");
    for (int i = 0; i < facultyCount; i++) {
        float util = (faculties[i].maxHours > 0) ? 
                     (faculties[i].assignedHours * 100.0 / faculties[i].maxHours) : 0;
        printf("%s: %d/%d hours (%.1f%% utilized)", 
               faculties[i].name, faculties[i].assignedHours, faculties[i].maxHours, util);
        
        if (util < 50.0 && faculties[i].maxHours > 0) {
            printf(" ⚠️  UNDERUTILIZED");
        } else if (util >= 100.0) {
            printf(" ✓ FULL");
        }
        printf("\n");
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
        fprintf(fp, "%d,\"%s\",%d,%d,%.2f\n", 
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
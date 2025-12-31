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

// Structures
typedef struct {
    int id;
    char name[MAX_NAME];
    int maxHours;
    int assignedHours;
} Faculty;

// CHANGED: Added sectionFacultyMap to support different faculty per section
typedef struct {
    int id;
    char name[MAX_NAME];
    int hoursPerWeek;
    int isLab;
    int labHours;
    char sections[MAX_SECTIONS][10];        // Section names (A, B, C, etc.)
    int facultyIds[MAX_SECTIONS];           // CHANGED: Faculty ID per section
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

// Global Variables
Faculty faculties[MAX_FACULTY];
Subject subjects[MAX_SUBJECTS];
Branch branch;
DaySlot days[MAX_DAYS];
TimeSlot timetable[MAX_DAYS][MAX_PERIODS][MAX_SECTIONS];
int facultyCount = 0, subjectCount = 0, dayCount = 0;

// Utility Functions
void trim(char* str) {
    char* start = str;
    while (*start == ' ' || *start == '\t' || *start == '\n' || *start == '\r') start++;
    if (*start == 0) { *str = 0; return; }
    char* end = start + strlen(start) - 1;
    while (end > start && (*end == ' ' || *end == '\t' || *end == '\n' || *end == '\r')) end--;
    *(end + 1) = 0;
    if (start != str) memmove(str, start, strlen(start) + 1);
}

int getSectionIndex(char* section) {
    for (int i = 0; i < branch.sectionCount; i++) {
        if (strcmp(branch.sections[i], section) == 0) return i;
    }
    return -1;
}

// CHANGED: New function to get faculty ID for a specific subject and section
int getFacultyForSection(int subjectId, char* section) {
    for (int i = 0; i < subjectCount; i++) {
        if (subjects[i].id == subjectId) {
            // Find the section index in this subject
            for (int j = 0; j < subjects[i].sectionCount; j++) {
                if (strcmp(subjects[i].sections[j], section) == 0) {
                    return subjects[i].facultyIds[j];
                }
            }
        }
    }
    return -1; // Not found
}

// CSV Reading Functions
void readFacultyCSV(const char* filename) {
    FILE* fp = fopen(filename, "r");
    if (!fp) { printf("Error: Cannot open %s\n", filename); return; }
    
    char line[MAX_LINE];
    fgets(line, MAX_LINE, fp);
    
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

// CHANGED: Updated to parse section-faculty mapping
void readSubjectsCSV(const char* filename) {
    FILE* fp = fopen(filename, "r");
    if (!fp) { printf("Error: Cannot open %s\n", filename); return; }
    
    char line[MAX_LINE];
    fgets(line, MAX_LINE, fp); // Skip header
    
    while (fgets(line, MAX_LINE, fp)) {
        line[strcspn(line, "\r\n")] = 0;
        
        if (strlen(line) == 0) continue;
        
        char* token = strtok(line, ",");
        if (!token) continue;
        subjects[subjectCount].id = atoi(token);
        
        token = strtok(NULL, ",");
        if (!token) continue;
        strcpy(subjects[subjectCount].name, token);
        trim(subjects[subjectCount].name);
        
        token = strtok(NULL, ",");
        if (!token) continue;
        subjects[subjectCount].hoursPerWeek = atoi(token);
        
        token = strtok(NULL, ",");
        if (!token) continue;
        subjects[subjectCount].isLab = atoi(token);
        
        if (subjects[subjectCount].isLab) {
            subjects[subjectCount].labHours = 1;
        } else {
            subjects[subjectCount].labHours = 0;
        }
        
        // CHANGED: Parse sectionFacultyMap (format: A:101;B:102;C:103)
        token = strtok(NULL, ",");
        if (token != NULL) {
            trim(token);
            char mapBuffer[MAX_LINE];
            strcpy(mapBuffer, token);
            
            // Parse each section:faculty pair
            char* pairToken = strtok(mapBuffer, ";");
            subjects[subjectCount].sectionCount = 0;
            
            while (pairToken != NULL && subjects[subjectCount].sectionCount < MAX_SECTIONS) {
                trim(pairToken);
                
                // Split by colon to get section and faculty
                char* colonPos = strchr(pairToken, ':');
                if (colonPos != NULL) {
                    *colonPos = '\0'; // Split the string
                    char* sectionName = pairToken;
                    char* facultyIdStr = colonPos + 1;
                    
                    trim(sectionName);
                    trim(facultyIdStr);
                    
                    // Store section name and faculty ID
                    strcpy(subjects[subjectCount].sections[subjects[subjectCount].sectionCount], sectionName);
                    subjects[subjectCount].facultyIds[subjects[subjectCount].sectionCount] = atoi(facultyIdStr);
                    subjects[subjectCount].sectionCount++;
                }
                
                pairToken = strtok(NULL, ";");
            }
        }
        
        // Debug output
        printf("Loaded: %s | Theory=%d hrs | Lab=%s | Section-Faculty Map: ",
               subjects[subjectCount].name, 
               subjects[subjectCount].hoursPerWeek,
               subjects[subjectCount].isLab ? "Yes" : "No");
        
        for (int i = 0; i < subjects[subjectCount].sectionCount; i++) {
            printf("%s:F%d%s", 
                   subjects[subjectCount].sections[i],
                   subjects[subjectCount].facultyIds[i],
                   i < subjects[subjectCount].sectionCount-1 ? ", " : "");
        }
        printf("\n");
        
        subjectCount++;
    }
    fclose(fp);
    printf("\nLoaded %d subjects total\n", subjectCount);
}

void readSectionsCSV(const char* filename) {
    FILE* fp = fopen(filename, "r");
    if (!fp) { printf("Error: Cannot open %s\n", filename); return; }
    
    char line[MAX_LINE];
    fgets(line, MAX_LINE, fp);
    
    if (fgets(line, MAX_LINE, fp)) {
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
    if (!fp) { printf("Error: Cannot open %s\n", filename); return; }
    
    char line[MAX_LINE];
    fgets(line, MAX_LINE, fp);
    
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

// Constraint Checking Functions
bool isFacultyFree(int facultyId, int day, int period) {
    for (int s = 0; s < branch.sectionCount; s++) {
        if (timetable[day][period][s].facultyId == facultyId) return false;
    }
    return true;
}

bool isFacultyFreeForLab(int facultyId, int day, int period) {
    if (!isFacultyFree(facultyId, day, period)) return false;
    if (period + 1 >= days[day].periods) return false;
    if (!isFacultyFree(facultyId, day, period + 1)) return false;
    return true;
}

bool hasLabOnDay(int day, int sectionIdx) {
    for (int p = 0; p < days[day].periods; p++) {
        if (timetable[day][p][sectionIdx].subjectId != -1) {
            for (int i = 0; i < subjectCount; i++) {
                if (subjects[i].id == timetable[day][p][sectionIdx].subjectId && subjects[i].isLab) {
                    return true;
                }
            }
        }
    }
    return false;
}

bool hasSameSubjectConsecutive(int subjectId, int day, int period, int sectionIdx) {
    if (period > 0 && timetable[day][period-1][sectionIdx].subjectId == subjectId) {
        return true;
    }
    if (period < days[day].periods - 1 && timetable[day][period+1][sectionIdx].subjectId == subjectId) {
        return true;
    }
    return false;
}

bool canAssign(int facultyId, int subjectId, int day, int period, int sectionIdx) {
    if (timetable[day][period][sectionIdx].facultyId != -1) return false;
    if (!isFacultyFree(facultyId, day, period)) return false;
    
    if (hasSameSubjectConsecutive(subjectId, day, period, sectionIdx)) return false;
    
    for (int f = 0; f < facultyCount; f++) {
        if (faculties[f].id == facultyId) {
            if (faculties[f].assignedHours >= faculties[f].maxHours) return false;
            break;
        }
    }
    return true;
}

bool canAssignLab(int facultyId, int day, int period, int sectionIdx) {
    if (timetable[day][period][sectionIdx].facultyId != -1) return false;
    if (period + 1 >= days[day].periods) return false;
    if (timetable[day][period + 1][sectionIdx].facultyId != -1) return false;
    if (!isFacultyFreeForLab(facultyId, day, period)) return false;
    
    if (hasLabOnDay(day, sectionIdx)) return false;
    
    for (int f = 0; f < facultyCount; f++) {
        if (faculties[f].id == facultyId) {
            if (faculties[f].assignedHours + 2 > faculties[f].maxHours) return false;
            break;
        }
    }
    return true;
}

int countClassesInDay(int day, int sectionIdx) {
    int count = 0;
    for (int p = 0; p < days[day].periods; p++) {
        if (timetable[day][p][sectionIdx].facultyId != -1) count++;
    }
    return count;
}

// CHANGED: Updated to use section-specific faculty
bool findAndAssignLabSlot(int subjectId, char* section, int* assignedDay, int* assignedPeriod) {
    int sectionIdx = getSectionIndex(section);
    if (sectionIdx == -1) return false;
    
    int subIdx = -1;
    for (int i = 0; i < subjectCount; i++) {
        if (subjects[i].id == subjectId) { subIdx = i; break; }
    }
    if (subIdx == -1) return false;
    
    // CHANGED: Get faculty ID for this specific section
    int facultyId = getFacultyForSection(subjectId, section);
    if (facultyId == -1) {
        printf("Error: No faculty assigned for subject %d, section %s\n", subjectId, section);
        return false;
    }
    
    int facIdx = -1;
    for (int i = 0; i < facultyCount; i++) {
        if (faculties[i].id == facultyId) { facIdx = i; break; }
    }
    if (facIdx == -1) return false;
    
    int bestDay = -1, bestPeriod = -1, minLoad = 9999;
    for (int d = 0; d < dayCount; d++) {
        int dayLoad = countClassesInDay(d, sectionIdx);
        for (int p = 0; p < days[d].periods - 1; p++) {
            if (canAssignLab(facultyId, d, p, sectionIdx)) {
                if (dayLoad < minLoad) {
                    minLoad = dayLoad;
                    bestDay = d;
                    bestPeriod = p;
                }
            }
        }
    }
    
    if (bestDay == -1) return false;
    
    timetable[bestDay][bestPeriod][sectionIdx].facultyId = facultyId;
    timetable[bestDay][bestPeriod][sectionIdx].subjectId = subjects[subIdx].id;
    strcpy(timetable[bestDay][bestPeriod][sectionIdx].section, section);
    
    timetable[bestDay][bestPeriod + 1][sectionIdx].facultyId = facultyId;
    timetable[bestDay][bestPeriod + 1][sectionIdx].subjectId = subjects[subIdx].id;
    strcpy(timetable[bestDay][bestPeriod + 1][sectionIdx].section, section);
    
    faculties[facIdx].assignedHours += 2;
    
    *assignedDay = bestDay;
    *assignedPeriod = bestPeriod;
    return true;
}

// CHANGED: Updated to use section-specific faculty
bool findAndAssignSlot(int subjectId, char* section, int* assignedDay, int* assignedPeriod) {
    int sectionIdx = getSectionIndex(section);
    if (sectionIdx == -1) return false;
    
    int subIdx = -1;
    for (int i = 0; i < subjectCount; i++) {
        if (subjects[i].id == subjectId) { subIdx = i; break; }
    }
    if (subIdx == -1) return false;
    
    // CHANGED: Get faculty ID for this specific section
    int facultyId = getFacultyForSection(subjectId, section);
    if (facultyId == -1) {
        printf("Error: No faculty assigned for subject %d, section %s\n", subjectId, section);
        return false;
    }
    
    int facIdx = -1;
    for (int i = 0; i < facultyCount; i++) {
        if (faculties[i].id == facultyId) { facIdx = i; break; }
    }
    if (facIdx == -1) return false;
    
    int bestDay = -1, bestPeriod = -1, minLoad = 9999;
    for (int d = 0; d < dayCount; d++) {
        int dayLoad = countClassesInDay(d, sectionIdx);
        
        for (int p = 0; p < days[d].periods; p++) {
            if (canAssign(facultyId, subjects[subIdx].id, d, p, sectionIdx)) {
                if (dayLoad < minLoad) {
                    minLoad = dayLoad;
                    bestDay = d;
                    bestPeriod = p;
                }
            }
        }
    }
    
    if (bestDay == -1) return false;
    
    timetable[bestDay][bestPeriod][sectionIdx].facultyId = facultyId;
    timetable[bestDay][bestPeriod][sectionIdx].subjectId = subjects[subIdx].id;
    strcpy(timetable[bestDay][bestPeriod][sectionIdx].section, section);
    
    faculties[facIdx].assignedHours++;
    
    *assignedDay = bestDay;
    *assignedPeriod = bestPeriod;
    return true;
}

void generateTimetable() {
    for (int d = 0; d < dayCount; d++) {
        for (int p = 0; p < days[d].periods; p++) {
            for (int s = 0; s < branch.sectionCount; s++) {
                timetable[d][p][s].facultyId = -1;
                timetable[d][p][s].subjectId = -1;
                strcpy(timetable[d][p][s].section, "");
            }
        }
    }
    
    // UPDATED: Track remaining hours for each subject per section
    // Format: remainingHours[subjectIndex][sectionIndex]
    int remainingHours[MAX_SUBJECTS][MAX_SECTIONS];
    for (int i = 0; i < subjectCount; i++) {
        for (int j = 0; j < subjects[i].sectionCount; j++) {
            remainingHours[i][j] = subjects[i].hoursPerWeek;
        }
    }
    
    printf("\n=== PHASE 1: Assigning Labs (Deducted from total hours) ===\n");
    int labsAssigned = 0;
    
    for (int i = 0; i < subjectCount; i++) {
        if (subjects[i].isLab) {
            for (int j = 0; j < subjects[i].sectionCount; j++) {
                int assignedDay = -1, assignedPeriod = -1;
                if (findAndAssignLabSlot(subjects[i].id, subjects[i].sections[j], &assignedDay, &assignedPeriod)) {
                    labsAssigned++;
                    
                    // UPDATED: Deduct 2 hours (lab) from total hoursPerWeek
                    remainingHours[i][j] -= 2;
                    
                    int facultyId = getFacultyForSection(subjects[i].id, subjects[i].sections[j]);
                    char* facultyName = "Unknown";
                    for (int f = 0; f < facultyCount; f++) {
                        if (faculties[f].id == facultyId) {
                            facultyName = faculties[f].name;
                            break;
                        }
                    }
                    printf("✓ LAB %d: %s - Section %s (Faculty: %s): Day %d, Periods %d-%d | Remaining theory hours: %d\n",
                           labsAssigned, subjects[i].name, subjects[i].sections[j], 
                           facultyName, assignedDay+1, assignedPeriod+1, assignedPeriod+2, remainingHours[i][j]);
                } else {
                    printf("✗ Failed LAB: %s - Section %s (no slot available)\n", subjects[i].name, subjects[i].sections[j]);
                }
            }
        }
    }
    
    printf("\n=== PHASE 2: Assigning Theory Classes (Remaining hours after lab deduction) ===\n");
    int theoryAssigned = 0;
    
    for (int i = 0; i < subjectCount; i++) {
        for (int j = 0; j < subjects[i].sectionCount; j++) {
            // UPDATED: Use remainingHours instead of hoursPerWeek
            int theoryHoursToAssign = remainingHours[i][j];
            
            if (theoryHoursToAssign > 0) {
                for (int h = 0; h < theoryHoursToAssign; h++) {
                    int assignedDay = -1, assignedPeriod = -1;
                    if (findAndAssignSlot(subjects[i].id, subjects[i].sections[j], &assignedDay, &assignedPeriod)) {
                        theoryAssigned++;
                        if (theoryAssigned <= 20 || theoryAssigned % 10 == 0) {
                            int facultyId = getFacultyForSection(subjects[i].id, subjects[i].sections[j]);
                            char* facultyName = "Unknown";
                            for (int f = 0; f < facultyCount; f++) {
                                if (faculties[f].id == facultyId) {
                                    facultyName = faculties[f].name;
                                    break;
                                }
                            }
                            printf("✓ Theory %d: %s - Section %s (Faculty: %s): Day %d, Period %d (hour %d/%d)\n",
                                   theoryAssigned, subjects[i].name, subjects[i].sections[j], 
                                   facultyName, assignedDay+1, assignedPeriod+1, h+1, theoryHoursToAssign);
                        }
                    } else {
                        printf("✗ Failed: %s - Section %s (hour %d/%d) - no valid slot\n", 
                               subjects[i].name, subjects[i].sections[j], h+1, theoryHoursToAssign);
                    }
                }
            }
        }
    }
    
    printf("\n=== Summary ===\n");
    printf("Labs assigned: %d (each lab = 2 periods)\n", labsAssigned);
    printf("Theory assigned: %d\n", theoryAssigned);
    printf("Total periods used: %d\n", (labsAssigned * 2) + theoryAssigned);
    
    printf("\n=== Constraint Validation ===\n");
    for (int s = 0; s < branch.sectionCount; s++) {
        printf("\nSection %s:\n", branch.sections[s]);
        for (int d = 0; d < dayCount; d++) {
            int dayClasses = countClassesInDay(d, s);
            int labCount = 0;
            
            for (int p = 0; p < days[d].periods; p++) {
                if (timetable[d][p][s].subjectId != -1) {
                    for (int i = 0; i < subjectCount; i++) {
                        if (subjects[i].id == timetable[d][p][s].subjectId && subjects[i].isLab) {
                            labCount++;
                            break;
                        }
                    }
                }
            }
            
            printf("  Day %d: %d classes, %d labs", d+1, dayClasses, labCount);
            if (labCount > 1) printf(" ⚠️ VIOLATION: Multiple labs!");
            if (dayClasses < 5) printf(" ⚠️ Less than 5 classes");
            printf("\n");
        }
    }
}

// ============================================================================
// UPDATED: Generate horizontal grid-style timetable (one per section)
// Format: Rows = Days, Columns = Periods
// ============================================================================
void generateSectionTimetable() {
    FILE* fp = fopen("section_timetable.csv", "w");
    if (!fp) {
        printf("Error: Cannot create section_timetable.csv\n");
        return;
    }
    
    // Find maximum number of periods across all days
    int maxPeriods = 0;
    for (int d = 0; d < dayCount; d++) {
        if (days[d].periods > maxPeriods) {
            maxPeriods = days[d].periods;
        }
    }
    
    // Generate timetable for each section
    for (int s = 0; s < branch.sectionCount; s++) {
        // Section header
        fprintf(fp, "Section %s\n", branch.sections[s]);
        
        // Header row: Day/Period, P1, P2, P3, ...
        fprintf(fp, "Day/Period");
        for (int p = 0; p < maxPeriods; p++) {
            fprintf(fp, ",P%d", p + 1);
        }
        fprintf(fp, "\n");
        
        // Generate rows for each day
        for (int d = 0; d < dayCount; d++) {
            // Day label in first column
            fprintf(fp, "Day %d", d + 1);
            
            // Generate cells for each period
            for (int p = 0; p < maxPeriods; p++) {
                fprintf(fp, ",");
                
                // Check if this period exists for this day
                if (p >= days[d].periods) {
                    fprintf(fp, "--");
                    continue;
                }
                
                // Check if slot is assigned
                if (timetable[d][p][s].facultyId == -1) {
                    fprintf(fp, "--");
                } else {
                    // Get subject name
                    char subName[MAX_NAME] = "Unknown";
                    char facName[MAX_NAME] = "Unknown";
                    int subjectId = timetable[d][p][s].subjectId;
                    bool isLabSubject = false;
                    
                    // Find subject name
                    for (int i = 0; i < subjectCount; i++) {
                        if (subjects[i].id == subjectId) {
                            strcpy(subName, subjects[i].name);
                            isLabSubject = subjects[i].isLab;
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
                    
                    // Check if this is a LAB slot
                    bool isLabSlot = false;
                    if (isLabSubject) {
                        // Check if next period has same subject (start of lab)
                        if (p + 1 < days[d].periods && 
                            timetable[d][p+1][s].subjectId == subjectId && 
                            timetable[d][p+1][s].facultyId == timetable[d][p][s].facultyId) {
                            isLabSlot = true;
                        }
                        // Check if previous period has same subject (continuation of lab)
                        else if (p > 0 && 
                                 timetable[d][p-1][s].subjectId == subjectId && 
                                 timetable[d][p-1][s].facultyId == timetable[d][p][s].facultyId) {
                            isLabSlot = true;
                        }
                    }
                    
                    // Format the cell content: "Subject (Faculty)" or "Subject LAB (Faculty)"
                    if (isLabSlot) {
                        fprintf(fp, "\"%s LAB (%s)\"", subName, facName);
                    } else {
                        fprintf(fp, "\"%s (%s)\"", subName, facName);
                    }
                }
            }
            fprintf(fp, "\n");
        }
        
        // Add blank line between sections (except after last section)
        if (s < branch.sectionCount - 1) {
            fprintf(fp, "\n");
        }
    }
    
    fclose(fp);
    printf("Generated section_timetable.csv (horizontal grid format)\n");
}

void generateFacultyTimetable() {
    FILE* fp = fopen("faculty_timetable.csv", "w");
    fprintf(fp, "Faculty,Day,Period,Subject,Section,Type\n");
    
    for (int f = 0; f < facultyCount; f++) {
        for (int d = 0; d < dayCount; d++) {
            for (int p = 0; p < days[d].periods; p++) {
                for (int s = 0; s < branch.sectionCount; s++) {
                    if (timetable[d][p][s].facultyId == faculties[f].id) {
                        char subName[MAX_NAME] = "Unknown";
                        char type[10] = "Theory";
                        
                        int subjectId = timetable[d][p][s].subjectId;
                        bool isLabSubject = false;
                        
                        for (int i = 0; i < subjectCount; i++) {
                            if (subjects[i].id == subjectId) {
                                strcpy(subName, subjects[i].name);
                                isLabSubject = subjects[i].isLab;
                                break;
                            }
                        }
                        
                        // FIXED: Check if this is a LAB slot by checking BOTH directions
                        if (isLabSubject) {
                            // Check if next period has same subject (start of lab)
                            if (p + 1 < days[d].periods && 
                                timetable[d][p+1][s].subjectId == subjectId && 
                                timetable[d][p+1][s].facultyId == faculties[f].id) {
                                strcpy(type, "Lab");
                            }
                            // Check if previous period has same subject (continuation of lab)
                            else if (p > 0 && 
                                     timetable[d][p-1][s].subjectId == subjectId && 
                                     timetable[d][p-1][s].facultyId == faculties[f].id) {
                                strcpy(type, "Lab");
                            }
                        }
                        
                        fprintf(fp, "\"%s\",%d,%d,\"%s\",\"%s\",\"%s\"\n", 
                                faculties[f].name, d+1, p+1, subName, branch.sections[s], type);
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
    fprintf(fp, "FacultyID,FacultyName,MaxHours,AssignedHours,Utilization\n");
    
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

int main() {
    printf("=== Timetable Generator with Section-Specific Faculty Assignment ===\n\n");
    
    readFacultyCSV("faculty.csv");
    readSubjectsCSV("subjects.csv");
    readSectionsCSV("sections.csv");
    readSlotsCSV("slots.csv");
    
    printf("\n=== Generating Timetable ===\n");
    generateTimetable();
    
    printf("\n=== Generating Output Files ===\n");
    generateSectionTimetable();
    generateFacultyTimetable();
    generateSummary();
    
    printf("\n=== Complete! ===\n");
    return 0;
}
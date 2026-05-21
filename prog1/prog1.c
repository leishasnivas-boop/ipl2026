#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    int id;
    char name[50];
    float gpa;
} Student;

// Function prototypes
void createUserRecords(const char *filename, int n);
void displayMthRecord(const char *filename, int m);
void deleteRecord(const char *filename, int idToDelete);
void displayAllRecords(const char *filename);
void clearInputBuffer();

int main() {
    const char *filename = "students.dat";
    int n, m, idToDelete;

    // 1. Get total number of records from user
    printf("Enter the number of records (n) you want to store: ");
    if (scanf("%d", &n) != 1 || n <= 0) {
        printf("Invalid number of records.\n");
        return 1;
    }
    clearInputBuffer(); // Clean buffer after reading integer

    // 2. Input data and save to file
    createUserRecords(filename, n);
    displayAllRecords(filename);

    // 3. Get specific m-th record from user
    printf("\nEnter the index of the record (m) you want to display (0 to %d): ", n - 1);
    if (scanf("%d", &m) == 1) {
        displayMthRecord(filename, m);
    }
    clearInputBuffer();

    // 4. Get ID to delete from user
    printf("\nEnter the Student ID you want to delete: ");
    if (scanf("%d", &idToDelete) == 1) {
        deleteRecord(filename, idToDelete);
    }
    clearInputBuffer();

    // 5. Display final contents
    printf("\n--- Final File Contents After Deletion ---\n");
    displayAllRecords(filename);

    return 0;
}

// Helper function to clear leftover newlines from scanf
void clearInputBuffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

// 1. Take N records from the user and store them in a binary file
void createUserRecords(const char *filename, int n) {
    FILE *file = fopen(filename, "wb"); 
    if (!file) {
        perror("Error opening file for writing");
        return;
    }

    Student s;
    printf("\n--- Enter Details for %d Students ---\n", n);
    for (int i = 0; i < n; i++) {
        printf("\nStudent %d:\n", i + 1);
        
        printf("Enter ID (integer): ");
        scanf("%d", &s.id);
        clearInputBuffer();

        printf("Enter Name: ");
        // Reads up to 49 characters or until a newline is hit
        scanf("%49[^\n]", s.name); 
        clearInputBuffer();

        printf("Enter GPA (float): ");
        scanf("%f", &s.gpa);
        clearInputBuffer();

        // Write this single record straight to the file
        fwrite(&s, sizeof(Student), 1, file);
    }

    fclose(file);
    printf("\nSuccessfully saved all %d records to %s\n", n, filename);
}

// 2. Get m-th record using fseek and display it
void displayMthRecord(const char *filename, int m) {
    FILE *file = fopen(filename, "rb"); 
    if (!file) {
        perror("Error opening file for reading");
        return;
    }

    Student s;
    int offset = m * sizeof(Student);

    // Jump straight to the memory location of the m-th index
    if (fseek(file, offset, SEEK_SET) != 0) {
        printf("Error: Could not seek to record index %d\n", m);
        fclose(file);
        return;
    }

    if (fread(&s, sizeof(Student), 1, file) == 1) {
        printf("\n[SUCCESS] Found Record at index %d:\n", m);
        printf("ID: %d | Name: %s | GPA: %.2f\n", s.id, s.name, s.gpa);
    } else {
        printf("\n[ERROR] Record index %d does not exist in the file.\n", m);
    }

    fclose(file);
}

// 3. Delete a record based on ID (The Shift Method)
void deleteRecord(const char *filename, int idToDelete) {
    FILE *file = fopen(filename, "r+b"); 
    if (!file) {
        perror("Error opening file for deletion");
        return;
    }

    Student current;
    int foundIndex = -1;
    int index = 0;

    // Step A: Search for the ID position
    while (fread(&current, sizeof(Student), 1, file) == 1) {
        if (current.id == idToDelete) {
            foundIndex = index;
            break;
        }
        index++;
    }

    if (foundIndex == -1) {
        printf("Record with ID %d not found.\n", idToDelete);
        fclose(file);
        return;
    }

    // Step B: Shift subsequent records forward to overwrite the target
    Student nextRecord;
    int currentReadIndex = foundIndex + 1;

    while (1) {
        fseek(file, currentReadIndex * sizeof(Student), SEEK_SET);
        if (fread(&nextRecord, sizeof(Student), 1, file) != 1) {
            break; 
        }

        fseek(file, (currentReadIndex - 1) * sizeof(Student), SEEK_SET);
        fwrite(&nextRecord, sizeof(Student), 1, file);

        currentReadIndex++;
    }

    // Step C: Shrink the file size physically down by 1 record
    fseek(file, 0, SEEK_END);
    long totalSize = ftell(file);
    int totalRecords = totalSize / sizeof(Student);
    int remainingRecords = totalRecords - 1;

    // Read remaining records into memory
    fseek(file, 0, SEEK_SET);
    Student *buffer = malloc(remainingRecords * sizeof(Student));
    if (buffer) {
        fread(buffer, sizeof(Student), remainingRecords, file);
    }
    fclose(file);

    // Overwrite the file with the truncated version
    if (buffer) {
        file = fopen(filename, "wb");
        fwrite(buffer, sizeof(Student), remainingRecords, file);
        fclose(file);
        free(buffer);
        printf("Record with ID %d successfully deleted.\n", idToDelete);
    }
}

// Utility to print everything currently in the file
void displayAllRecords(const char *filename) {
    FILE *file = fopen(filename, "rb");
    if (!file) return;

    Student s;
    int idx = 0;
    printf("\nCurrent File Content:\n");
    printf("----------------------------------------\n");
    while (fread(&s, sizeof(Student), 1, file) == 1) {
        printf("Index %d | ID: %d | Name: %-15s | GPA: %.2f\n", idx++, s.id, s.name, s.gpa);
    }
    printf("----------------------------------------\n");
    fclose(file);
}
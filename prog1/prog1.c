#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Define a fixed-size structure
typedef struct {
    int id;
    char name[50];
    float gpa;
} Student;

// Function prototypes
void createFile(const char *filename, int n);
void displayMthRecord(const char *filename, int m);
void deleteRecord(const char *filename, int idToDelete);
void displayAllRecords(const char *filename);

int main() {
    const char *filename = "students.dat";
    int n = 5;

    printf("--- Step 1: Creating file with %d records ---\n", n);
    createFile(filename, n);
    displayAllRecords(filename);

    printf("\n--- Step 2: Fetching the 3rd record (0-indexed, so index 2) using fseek ---\n");
    displayMthRecord(filename, 2);

    printf("\n--- Step 3: Deleting Student with ID 103 ---\n");
    deleteRecord(filename, 103);

    printf("\n--- Step 4: Final File Content ---\n");
    displayAllRecords(filename);

    return 0;
}

// 1. Store N records in a binary file
void createFile(const char *filename, int n) {
    FILE *file = fopen(filename, "wb"); // 'wb' opens for writing in binary mode
    if (!file) {
        perror("Error opening file for writing");
        return;
    }

    // Hardcoding some sample data for demonstration
    Student records[5] = {
        {101, "Alice", 3.8},
        {102, "Bob", 3.4},
        {103, "Charlie", 3.9},
        {104, "David", 2.9},
        {105, "Eva", 3.7}
    };

    // Write the array of structures directly to the binary file
    size_t written = fwrite(records, sizeof(Student), n, file);
    printf("Successfully wrote %zu records to %s\n", written, filename);
    
    fclose(file);
}

// 2. Get m-th record using fseek and display it
void displayMthRecord(const char *filename, int m) {
    FILE *file = fopen(filename, "rb"); // 'rb' opens for reading in binary mode
    if (!file) {
        perror("Error opening file for reading");
        return;
    }

    Student s;
    // Offset calculation: index * size of one structure
    // SEEK_SET tells fseek to measure the offset from the beginning of the file
    int offset = m * sizeof(Student);

    if (fseek(file, offset, SEEK_SET) != 0) {
        printf("Error: Could not seek to record index %d\n", m);
        fclose(file);
        return;
    }

    // Read exactly one record from the seeked position
    if (fread(&s, sizeof(Student), 1, file) == 1) {
        printf("Record at index %d -> ID: %d, Name: %s, GPA: %.2f\n", m, s.id, s.name, s.gpa);
    } else {
        printf("Error: Could not read record at index %d (Out of bounds?)\n", m);
    }

    fclose(file);
}

// 3. Delete a record (The Shift Method)
void deleteRecord(const char *filename, int idToDelete) {
    // Open in read+update binary mode
    FILE *file = fopen(filename, "r+b"); 
    if (!file) {
        perror("Error opening file for deletion");
        return;
    }

    Student current;
    int foundIndex = -1;
    int index = 0;

    // Step A: Find the record to delete
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

    // Step B: Shift all subsequent records one slot up
    Student nextRecord;
    int currentReadIndex = foundIndex + 1;

    while (1) {
        // Seek to the record we want to pull forward
        fseek(file, currentReadIndex * sizeof(Student), SEEK_SET);
        
        // If there's nothing left to read, we've hit the end of the file
        if (fread(&nextRecord, sizeof(Student), 1, file) != 1) {
            break; 
        }

        // Seek back to the overwrite position (one slot back)
        fseek(file, (currentReadIndex - 1) * sizeof(Student), SEEK_SET);
        fwrite(&nextRecord, sizeof(Student), 1, file);

        currentReadIndex++;
    }

    // Step C: Truncate the file to remove the trailing duplicate record
    // Since standard C doesn't have a cross-platform truncate function, 
    // the cleanest standard C way is to calculate the new size, close the file, 
    // and let the system know the file is now shorter if we were doing a rewrite,
    // OR we can use platform-specific tricks. 
    // An alternative approach to keep standard C perfectly clean without system headers:
    
    // Let's count total remaining records
    fseek(file, 0, SEEK_END);
    long totalSize = ftell(file);
    int totalRecords = totalSize / sizeof(Student);
    int remainingRecords = totalRecords - 1;

    // Read remaining valid data into a temp buffer to rewrite cleanly
    fseek(file, 0, SEEK_SET);
    Student *buffer = malloc(remainingRecords * sizeof(Student));
    fread(buffer, sizeof(Student), remainingRecords, file);
    
    fclose(file);

    // Reopen with "wb" to wipe and rewrite only the valid records
    file = fopen(filename, "wb");
    fwrite(buffer, sizeof(Student), remainingRecords, file);
    fclose(file);
    free(buffer);

    printf("Record with ID %d successfully deleted.\n", idToDelete);
}

// Helper function to display everything
void displayAllRecords(const char *filename) {
    FILE *file = fopen(filename, "rb");
    if (!file) return;

    Student s;
    int idx = 0;
    printf("\nCurrent Database Records:\n");
    printf("----------------------------------------\n");
    while (fread(&s, sizeof(Student), 1, file) == 1) {
        printf("Index %d | ID: %d | Name: %-10s | GPA: %.2f\n", idx++, s.id, s.name, s.gpa);
    }
    printf("----------------------------------------\n");
    fclose(file);
}

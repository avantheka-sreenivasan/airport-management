 //use this command in terminal to run "gcc `pkg-config --cflags --libs gtk+-3.0` PROJECT_FINAL.c"
 //install gtk 3+ before running
 #include <stdio.h>
 #include <stdlib.h>
 #include <string.h>
 #include <time.h>
 #include <stdbool.h>
 #include <ctype.h>
 #include <gtk/gtk.h>
 #include <limits.h>
 
 #define MAX_FLIGHTS 100
 #define MAX_RUNWAYS 5
 #define MAX_CREW 30
 #define MAX_USERS 10
 #define BUFFER_TIME 15
 #define MAX_DUTY_TIME 480
 #define MIN_REST_TIME 60
 #define CREW_BUFFER_TIME 30
 #define STR_LEN 50
 #define MAX_ACTIONS 100
 #define NOTIFICATION_BUFFER 100
 #define FILENAME_FLIGHTS "flights.dat"
 #define FILENAME_RUNWAYS "runways.dat"
 #define FILENAME_CREW "crew.dat"
 #define FILENAME_USERS "users.dat"
 
 typedef enum {
     ADMIN,
     FLIGHT_SCHEDULER,
     CREW_SCHEDULER,
     VIEWER
 } UserRole;
 
 typedef enum {
     EMERGENCY = 1,
     INTERNATIONAL = 2,
     DOMESTIC = 3
 } FlightPriority;
 
 typedef enum {
     SCHEDULED,
     DELAYED,
     CANCELLED,
     EMERGENCY_STATUS
 } FlightStatus;
 
 typedef enum {
     ALL_FLIGHTS,
     INTERNATIONAL_ONLY,
     CARGO_ONLY
 } RunwayType;
 
 typedef struct {
     int hour;
     int minute;
 } Time;
 
 typedef struct {
     char flightID[STR_LEN];
     char origin[STR_LEN];
     char destination[STR_LEN];
     char aircraftType[STR_LEN];
     Time departureTime;
     Time arrivalTime;
     FlightPriority priority;
     FlightStatus status;
     int runwayAssigned;
     int crewAssigned;
     int delayMinutes;
     bool isCargo;
 } Flight;
 
 typedef struct {
     int id;
     RunwayType type;
     bool isAvailable;
     Time nextAvailableTime;
 } Runway;
 
 typedef struct {
     int id;
     char name[STR_LEN];
     int dutyMinutesToday;
     Time lastFlightEnd;
     bool isAvailable;
     char qualifications[STR_LEN];
 } Crew;
 
 typedef struct {
     char username[STR_LEN];
     char password[STR_LEN];
     UserRole role;
 } User;
 
 typedef struct {
     char description[STR_LEN];
     void (*undoFunction)(void*);
     void *data;
     size_t dataSize;
 } Action;
 
 typedef struct {
     char message[STR_LEN*2];
     Time timestamp;
     bool isWarning;
     bool isError;
 } Notification;
 
 Flight flights[MAX_FLIGHTS];
 Runway runways[MAX_RUNWAYS];
 Crew crews[MAX_CREW];
 User users[MAX_USERS];
 Notification notifications[NOTIFICATION_BUFFER];
 Action undoStack[MAX_ACTIONS];
 int flightCount = 0;
 int runwayCount = 0;
 int crewCount = 0;
 int userCount = 0;
 int notificationCount = 0;
 int undoCount = 0;
 User currentUser;
 int selectedFlightIndex = -1;
 
 GtkWidget *window;
 GtkWidget *stack;
 GtkWidget *login_grid;
 GtkWidget *main_menu_grid;
 GtkWidget *flight_management_grid;
 GtkWidget *runway_management_grid;
 GtkWidget *crew_management_grid;
 GtkWidget *realtime_events_grid;
 GtkWidget *schedule_grid;
 GtkWidget *search_flight_grid;
 GtkWidget *reports_grid;
 GtkWidget *logout_grid;
 GtkWidget *username_entry;
 GtkWidget *password_entry;
 GtkWidget *notification_label = NULL;
 GtkWidget *flight_list;
 GtkWidget *flight_details_text;
 GtkWidget *runway_list;
 GtkWidget *crew_list;
 GtkWidget *schedule_list;
 GtkWidget *search_entry;
 GtkWidget *search_results;
 GtkWidget *report_text;
 GtkWidget *flight_id_entry;
 GtkWidget *origin_entry;
 GtkWidget *destination_entry;
 GtkWidget *aircraft_entry;
 GtkWidget *dep_hour_spin;
 GtkWidget *dep_min_spin;
 GtkWidget *duration_spin;
 GtkWidget *priority_combo_text;
 GtkWidget *cargo_check;
 GtkWidget *delay_spin;
 
 void addNotification(const char* message, bool isWarning, bool isError);
 void update_flight_list(void);
 void update_runway_list(void);
 void update_crew_list(void);
 void update_schedule_list(void);
 
 Time addMinutesToTime(Time t, int minutes);
 int calculateTimeDifferenceInMinutes(Time t1, Time t2);
 bool isTimeAfter(Time t1, Time t2);
 Time getCurrentTime();
 void printTime(Time t);
 int compareTime(Time t1, Time t2);
 void clearInputBuffer();
 
 void initializeSystem();
 void initializeUsers();
 bool authenticateUser(const char* username, const char* password, User* user);
 void saveDataToFiles();
 void loadDataFromFiles();
 void addNotification(const char* message, bool isWarning, bool isError);
 
 void addFlight();
 void modifyFlight();
 void deleteFlight();
 bool validateFlightID(char* flightID);
 
 void assignRunways();
 
 void scheduleCrew();
 
 void handleWeatherDelay(int delayMinutes);
 void handleEmergencyLanding();
 void handleFlightCancellation();
 void rescheduleFlights();
 
 void on_login_clicked(GtkWidget *widget, gpointer data);
 void on_back_clicked(GtkWidget *widget, gpointer data);
 void on_logout_clicked(GtkWidget *widget, gpointer data);
 void on_add_flight_clicked(GtkWidget *widget, gpointer data);
 void on_modify_flight_clicked(GtkWidget *widget, gpointer data);
 void on_delete_flight_clicked(GtkWidget *widget, gpointer data);
 void on_assign_runways_clicked(GtkWidget *widget, gpointer data);
 void on_schedule_crew_clicked(GtkWidget *widget, gpointer data);
 void on_weather_delay_clicked(GtkWidget *widget, gpointer data);
 void on_emergency_landing_clicked(GtkWidget *widget, gpointer data);
 void on_flight_cancellation_clicked(GtkWidget *widget, gpointer data);
 void on_reschedule_clicked(GtkWidget *widget, gpointer data);
 void on_search_clicked(GtkWidget *widget, gpointer data);
 void on_generate_flights_report_clicked(GtkWidget *widget, gpointer data);
 void on_generate_runways_report_clicked(GtkWidget *widget, gpointer data);
 void on_generate_crew_report_clicked(GtkWidget *widget, gpointer data);
 void on_flight_selected(GtkTreeSelection *selection, gpointer data);
 void on_delay_dialog_response(GtkDialog *dialog, gint response_id, gpointer user_data);
 
 void update_flight_list();
 void update_runway_list();
 void update_crew_list();
 void update_schedule_list();
 void show_notification(const char *message);
 void switch_to_screen(GtkWidget *widget, gpointer data);
 void show_delay_dialog();

 void create_login_screen();
 void create_main_menu();
 void create_flight_management();
 void create_runway_management();
 void create_crew_management();
 void create_realtime_events();
 void create_schedule();
 void create_search_flight();
 void create_reports();
 void create_logout_screen();
 
 Time addMinutesToTime(Time t, int minutes) {
     Time result = t;
     result.minute += minutes;
     
     while (result.minute >= 60) {
         result.hour++;
         result.minute -= 60;
     }
     
     while (result.hour >= 24) {
         result.hour -= 24;
     }
     
     return result;
 }
 
 void switch_to_screen(GtkWidget *widget, gpointer data) {
    const char *screen_name = (const char *)data;
    
    bool has_access = false;
    
    if (strcmp(screen_name, "flight_management") == 0) {
        has_access = (currentUser.role == ADMIN || currentUser.role == FLIGHT_SCHEDULER);
    }
    else if (strcmp(screen_name, "runway_management") == 0) {
        has_access = (currentUser.role == ADMIN || currentUser.role == CREW_SCHEDULER);
    }
    else if (strcmp(screen_name, "crew_management") == 0) {
        has_access = (currentUser.role == ADMIN || currentUser.role == CREW_SCHEDULER);
    }
    else if (strcmp(screen_name, "realtime_events") == 0) {
        has_access = (currentUser.role == ADMIN);
    }
    else if (strcmp(screen_name, "schedule") == 0 || 
             strcmp(screen_name, "search_flight") == 0) {
        has_access = true; 
    }
    else if (strcmp(screen_name, "reports") == 0) {
        has_access = (currentUser.role == ADMIN || currentUser.role == VIEWER);
    }
    else if (strcmp(screen_name, "main_menu") == 0 || 
             strcmp(screen_name, "login") == 0 ||
             strcmp(screen_name, "logout") == 0) {
        has_access = true; 
    }
    
    if (!has_access) {
        gtk_label_set_text(GTK_LABEL(notification_label), "Access denied: You don't have permission to view this screen");
        return;
    }
    
    gtk_stack_set_visible_child_name(GTK_STACK(stack), screen_name);
    
    if (strcmp(screen_name, "flight_management") == 0) {
        update_flight_list();
    } else if (strcmp(screen_name, "runway_management") == 0) {
        update_runway_list();
    } else if (strcmp(screen_name, "crew_management") == 0) {
        update_crew_list();
    } else if (strcmp(screen_name, "schedule") == 0) {
        update_schedule_list();
    }
}
 
 int calculateTimeDifferenceInMinutes(Time t1, Time t2) {
     int t1Minutes = t1.hour * 60 + t1.minute;
     int t2Minutes = t2.hour * 60 + t2.minute;
     
     if (t2Minutes < t1Minutes) {
         t2Minutes += 24 * 60;
     }
     
     return t2Minutes - t1Minutes;
 }
 
 bool isTimeAfter(Time t1, Time t2) {
     if (t1.hour > t2.hour) return true;
     if (t1.hour == t2.hour && t1.minute > t2.minute) return true;
     return false;
 }
 
 Time getCurrentTime() {
     time_t rawtime;
     struct tm* timeinfo;
     time(&rawtime);
     timeinfo = localtime(&rawtime);
     
     Time currentTime;
     currentTime.hour = timeinfo->tm_hour;
     currentTime.minute = timeinfo->tm_min;
     
     return currentTime;
 }
 
 void printTime(Time t) {
     printf("%02d:%02d", t.hour, t.minute);
 }
 
 int compareTime(Time t1, Time t2) {
     if (t1.hour < t2.hour) return -1;
     if (t1.hour > t2.hour) return 1;
     if (t1.minute < t2.minute) return -1;
     if (t1.minute > t2.minute) return 1;
     return 0;
 }
 
 void clearInputBuffer() {
     while (getchar() != '\n');
 }
 
 
 void initializeSystem() {
     for (int i = 0; i < MAX_FLIGHTS; i++) {
        flights[i].runwayAssigned = -1;
        flights[i].crewAssigned = -1;
        flights[i].delayMinutes = 0;
        flights[i].status = SCHEDULED;
    }

    
    runwayCount = 3; 
    for (int i = 0; i < runwayCount; i++) {
        runways[i].id = i;
        runways[i].isAvailable = true;
        runways[i].nextAvailableTime.hour = 0;
        runways[i].nextAvailableTime.minute = 0;

        if (i == 0) {
            runways[i].type = ALL_FLIGHTS;
        } else if (i == 1) {
            runways[i].type = INTERNATIONAL_ONLY;
        } else if (i == 2) {
            runways[i].type = CARGO_ONLY;
        }
    }
     
    char* crewNames[] = {"Capt. Smith", "F/O Johnson", "Capt. Williams", 
                          "F/O Brown", "Capt. Davis", "F/O Miller", "Capt. Wilson",
                          "F/O Moore", "Capt. Taylor", "F/O Anderson"};
     char* qualifications[] = {"Boeing737,AirbusA320", "Boeing787,AirbusA350", 
                               "Boeing737,Embraer190", "AirbusA320,AirbusA380", 
                               "Boeing777,AirbusA350", "Boeing737,AirbusA320",
                               "Boeing787,AirbusA350", "Boeing747,AirbusA380",
                               "Embraer190,Embraer195", "Boeing737,AirbusA320"};
     
     for (int i = 0; i < 10; i++) {
         crews[i].id = i;
         strcpy(crews[i].name, crewNames[i]);
         crews[i].dutyMinutesToday = 0;
         crews[i].lastFlightEnd.hour = 0;
         crews[i].lastFlightEnd.minute = 0;
         crews[i].isAvailable = true;
         strcpy(crews[i].qualifications, qualifications[i]);
     }
     crewCount = 10;
     
     notificationCount = 0;
     undoCount = 0;
     
     addNotification("System initialized with 5 runways and 10 crew members", false, false);
 }
 
 void initializeUsers() {
    strcpy(users[0].username, "admin");
    strcpy(users[0].password, "admin123");
    users[0].role = ADMIN;
    
    strcpy(users[1].username, "flight");
    strcpy(users[1].password, "flight123");
    users[1].role = FLIGHT_SCHEDULER;
    
    strcpy(users[2].username, "crew");
    strcpy(users[2].password, "crew123");
    users[2].role = CREW_SCHEDULER;
    
    strcpy(users[3].username, "viewer");
    strcpy(users[3].password, "viewer123");
    users[3].role = VIEWER;
    
    userCount = 4;
}
 
bool authenticateUser(const char* username, const char* password, User* user) {
    printf("Attempting to authenticate: %s\n", username);
    
    for (int i = 0; i < userCount; i++) {
        printf("Checking user %d: %s\n", i, users[i].username);
        
        if (strcmp(users[i].username, username) == 0 && 
            strcmp(users[i].password, password) == 0) {
            *user = users[i];
            printf("Authentication successful for %s\n", username);
            return true;
        }
    }
    
    printf("Authentication failed for %s\n", username);
    return false;
}

 
 void saveDataToFiles() {
     FILE *file;
     
     file = fopen(FILENAME_FLIGHTS, "wb");
     if (file) {
         fwrite(&flightCount, sizeof(int), 1, file);
         fwrite(flights, sizeof(Flight), flightCount, file);
         fclose(file);
     }
     
     file = fopen(FILENAME_RUNWAYS, "wb");
     if (file) {
         fwrite(&runwayCount, sizeof(int), 1, file);
         fwrite(runways, sizeof(Runway), runwayCount, file);
         fclose(file);
     }
     
     file = fopen(FILENAME_CREW, "wb");
     if (file) {
         fwrite(&crewCount, sizeof(int), 1, file);
         fwrite(crews, sizeof(Crew), crewCount, file);
         fclose(file);
     }
     
     file = fopen(FILENAME_USERS, "wb");
     if (file) {
         fwrite(&userCount, sizeof(int), 1, file);
         fwrite(users, sizeof(User), userCount, file);
         fclose(file);
     }
     
     addNotification("Data saved to files", false, false);
 }
 
 void loadDataFromFiles() {
     FILE *file;
     
    
    file = fopen(FILENAME_FLIGHTS, "rb");
    if (file) {
        fread(&flightCount, sizeof(int), 1, file);
        if (flightCount > MAX_FLIGHTS) flightCount = MAX_FLIGHTS;
        fread(flights, sizeof(Flight), flightCount, file);
        fclose(file);
    }
    
    file = fopen(FILENAME_RUNWAYS, "rb");
    if (file) {
        fread(&runwayCount, sizeof(int), 1, file);
        if (runwayCount > MAX_RUNWAYS) runwayCount = MAX_RUNWAYS; 
        fread(runways, sizeof(Runway), runwayCount, file);
        fclose(file);
    } else {
        runwayCount = 3;
        for (int i = 0; i < runwayCount; i++) {
            runways[i].id = i;
            runways[i].isAvailable = true;
            runways[i].nextAvailableTime.hour = 0;
            runways[i].nextAvailableTime.minute = 0;
            if (i == 0) runways[i].type = ALL_FLIGHTS;
            else if (i == 1) runways[i].type = INTERNATIONAL_ONLY;
            else if (i == 2) runways[i].type = CARGO_ONLY;
        }
    }
     
     file = fopen(FILENAME_CREW, "rb");
     if (file) {
         fread(&crewCount, sizeof(int), 1, file);
         fread(crews, sizeof(Crew), crewCount, file);
         fclose(file);
     }
     
     file = fopen(FILENAME_USERS, "rb");
     if (file) {
         fread(&userCount, sizeof(int), 1, file);
         fread(users, sizeof(User), userCount, file);
         fclose(file);
     }
     
     addNotification("Data loaded from files", false, false);
 }
 
 void addNotification(const char* message, bool isWarning, bool isError) {
     if (notificationCount >= NOTIFICATION_BUFFER) {
         for (int i = 0; i < NOTIFICATION_BUFFER - 1; i++) {
             notifications[i] = notifications[i + 1];
         }
         notificationCount--;
     }
     
     strncpy(notifications[notificationCount].message, message, STR_LEN*2 - 1);
     notifications[notificationCount].timestamp = getCurrentTime();
     notifications[notificationCount].isWarning = isWarning;
     notifications[notificationCount].isError = isError;
     notificationCount++;
 }
 
 
 void addFlight() {
     if (flightCount >= MAX_FLIGHTS) {
         addNotification("Failed to add flight: Maximum limit reached", false, true);
         return;
     }
     

     const char *flightID = gtk_entry_get_text(GTK_ENTRY(flight_id_entry));
     const char *origin = gtk_entry_get_text(GTK_ENTRY(origin_entry));
     const char *destination = gtk_entry_get_text(GTK_ENTRY(destination_entry));
const char *aircraftType = NULL;
GtkTreeIter aircraft_iter;
if (gtk_combo_box_get_active_iter(GTK_COMBO_BOX(aircraft_entry), &aircraft_iter)) {
    GtkTreeModel *model = gtk_combo_box_get_model(GTK_COMBO_BOX(aircraft_entry));
    gtk_tree_model_get(model, &aircraft_iter, 0, &aircraftType, -1);
}
if (aircraftType == NULL) {
    addNotification("Please select an aircraft type", false, true);
    return;
}
     
     int dep_hour = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(dep_hour_spin));
     int dep_min = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(dep_min_spin));
     int duration = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(duration_spin));
     
     gint priority_index = gtk_combo_box_get_active(GTK_COMBO_BOX(priority_combo_text));
     
     bool isCargo = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(cargo_check));
     
     if (strlen(flightID) == 0 || strlen(origin) == 0 || strlen(destination) == 0 || strlen(aircraftType) == 0) {
         addNotification("Please fill in all flight details", false, true);
         return;
     }
     
     if (!validateFlightID((char*)flightID)) {
         addNotification("Flight ID already exists", false, true);
         return;
     }
     
     Flight newFlight;
     strncpy(newFlight.flightID, flightID, STR_LEN - 1);
     strncpy(newFlight.origin, origin, STR_LEN - 1);
     strncpy(newFlight.destination, destination, STR_LEN - 1);
     strncpy(newFlight.aircraftType, aircraftType, STR_LEN - 1);
     
     newFlight.departureTime.hour = dep_hour;
     newFlight.departureTime.minute = dep_min;
     newFlight.arrivalTime = addMinutesToTime(newFlight.departureTime, duration);
     newFlight.priority = (FlightPriority)(priority_index + 1);
     newFlight.status = SCHEDULED;
     newFlight.runwayAssigned = -1;
     newFlight.crewAssigned = -1;
     newFlight.delayMinutes = 0;
     newFlight.isCargo = isCargo;
     
     flights[flightCount] = newFlight;
     flightCount++;
     
     char msg[STR_LEN*2];
     sprintf(msg, "Flight %s added successfully", newFlight.flightID);
     addNotification(msg, false, false);
     
     gtk_entry_set_text(GTK_ENTRY(flight_id_entry), "");
     gtk_entry_set_text(GTK_ENTRY(origin_entry), "");
     gtk_entry_set_text(GTK_ENTRY(destination_entry), "");
     gtk_entry_set_text(GTK_ENTRY(aircraft_entry), "");
     gtk_spin_button_set_value(GTK_SPIN_BUTTON(dep_hour_spin), 0);
     gtk_spin_button_set_value(GTK_SPIN_BUTTON(dep_min_spin), 0);
     gtk_spin_button_set_value(GTK_SPIN_BUTTON(duration_spin), 0);
     gtk_combo_box_set_active(GTK_COMBO_BOX(priority_combo_text), 0);
     gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(cargo_check), FALSE);
 }
 
 void modifyFlight() {
     if (selectedFlightIndex == -1 || selectedFlightIndex >= flightCount) {
         addNotification("No flight selected for modification", false, true);
         return;
     }

     int delayMinutes = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(delay_spin));
     
     if (delayMinutes <= 0) {
         addNotification("Please enter a valid delay time", false, true);
         return;
     }
     
     flights[selectedFlightIndex].departureTime = 
         addMinutesToTime(flights[selectedFlightIndex].departureTime, delayMinutes);
     flights[selectedFlightIndex].arrivalTime = 
         addMinutesToTime(flights[selectedFlightIndex].arrivalTime, delayMinutes);
     flights[selectedFlightIndex].delayMinutes += delayMinutes;
     flights[selectedFlightIndex].status = DELAYED;
     
     char msg[STR_LEN*2];
     sprintf(msg, "Flight %s delayed by %d minutes", flights[selectedFlightIndex].flightID, delayMinutes);
     addNotification(msg, false, false);
     
     gtk_spin_button_set_value(GTK_SPIN_BUTTON(delay_spin), 0);
 }
 
 void deleteFlight() {
     if (selectedFlightIndex == -1 || selectedFlightIndex >= flightCount) {
         addNotification("No flight selected for deletion", false, true);
         return;
     }
     
     char flightID[STR_LEN];
     strcpy(flightID, flights[selectedFlightIndex].flightID);
     
     for (int i = selectedFlightIndex; i < flightCount - 1; i++) {
         flights[i] = flights[i + 1];
     }
     flightCount--;
     selectedFlightIndex = -1;
     
     char msg[STR_LEN*2];
     sprintf(msg, "Flight %s deleted", flightID);
     addNotification(msg, false, false);
 }
 
 bool validateFlightID(char* flightID) {
     for (int i = 0; i < flightCount; i++) {
         if (strcmp(flights[i].flightID, flightID) == 0) {
             return false;
         }
     }
     return true;
 }
 
 
 void assignRunways() {
    for (int i = 0; i < flightCount; i++) {
        if (flights[i].runwayAssigned == -1 && flights[i].status != CANCELLED) {
           
            for (int r = 0; r < runwayCount; r++) {
                
                bool isCompatible = false;
                
                if (runways[r].isAvailable) {
                    switch(runways[r].type) {
                        case ALL_FLIGHTS:
                            isCompatible = true;
                            break;
                        case INTERNATIONAL_ONLY:
                            isCompatible = (flights[i].priority == INTERNATIONAL);
                            break;
                        case CARGO_ONLY:
                            isCompatible = flights[i].isCargo;
                            break;
                    }
                }
                
           
                if (isCompatible && (compareTime(flights[i].departureTime, 
                                     runways[r].nextAvailableTime) >= 0)) {
                   
                    flights[i].runwayAssigned = r;
                    runways[r].isAvailable = false;
                    runways[r].nextAvailableTime = 
                        addMinutesToTime(flights[i].arrivalTime, BUFFER_TIME);
                    break; 
                }
            }
            
           
            if (flights[i].runwayAssigned == -1) {
                flights[i].status = DELAYED;
                flights[i].delayMinutes += 15; 
            }
        }
    }
}
 
 
void scheduleCrew() {
    for (int i = 0; i < flightCount; i++) {
        if (flights[i].crewAssigned == -1 && flights[i].status != CANCELLED) {
           
            int bestCrewIndex = -1;
            int bestScore = -1;
            
            for (int c = 0; c < crewCount; c++) {
                if (!crews[c].isAvailable) continue;
                if (crews[c].dutyMinutesToday >= MAX_DUTY_TIME) continue;
                Time earliestStart = addMinutesToTime(crews[c].lastFlightEnd, MIN_REST_TIME);
                if (compareTime(flights[i].departureTime, earliestStart) < 0) continue;
              
                char qualStr[STR_LEN*2];
                snprintf(qualStr, sizeof(qualStr), ",%s,", crews[c].qualifications);
                char aircraftStr[STR_LEN*2];
                snprintf(aircraftStr, sizeof(aircraftStr), ",%s,", flights[i].aircraftType);
                
                int score = 0;
                if (strstr(qualStr, aircraftStr) != NULL) {
                    score = 100; 
                } else if (strstr(flights[i].aircraftType, "Boeing") && 
                          strstr(qualStr, ",Boeing,")) {
                    score = 50; 
                } else if (strstr(flights[i].aircraftType, "Airbus") && 
                          strstr(qualStr, ",Airbus,")) {
                    score = 50; 
                }

                score += (MAX_DUTY_TIME - crews[c].dutyMinutesToday)/10;
                
                if (score > bestScore) {
                    bestScore = score;
                    bestCrewIndex = c;
                }
            }
            
            if (bestCrewIndex != -1) {
                flights[i].crewAssigned = bestCrewIndex;
                crews[bestCrewIndex].isAvailable = false;
                crews[bestCrewIndex].lastFlightEnd = flights[i].arrivalTime;
                
                int flightDuration = calculateTimeDifferenceInMinutes(
                    flights[i].departureTime, flights[i].arrivalTime);
                crews[bestCrewIndex].dutyMinutesToday += flightDuration;
            } else {
              
                flights[i].status = DELAYED;
                flights[i].delayMinutes += 30; 
            }

        }
    }
}
 
 
 void handleWeatherDelay(int delayMinutes) {
     if (flightCount == 0) {
         addNotification("No flights to delay", false, true);
         return;
     }
 
     int flightIndex = rand() % flightCount;
     
     flights[flightIndex].departureTime = 
         addMinutesToTime(flights[flightIndex].departureTime, delayMinutes);
     flights[flightIndex].arrivalTime = 
         addMinutesToTime(flights[flightIndex].arrivalTime, delayMinutes);
     flights[flightIndex].delayMinutes += delayMinutes;
     flights[flightIndex].status = DELAYED;
     
     char msg[STR_LEN*2];
     sprintf(msg, "Weather delay: Flight %s delayed by %d minutes", 
            flights[flightIndex].flightID, delayMinutes);
     addNotification(msg, true, false);
 }
 
 void handleEmergencyLanding() {
     if (flightCount >= MAX_FLIGHTS) {
         addNotification("Cannot add emergency flight - maximum reached", false, true);
         return;
     }
 
     Flight emergencyFlight;
     strcpy(emergencyFlight.flightID, "EMG");
     emergencyFlight.flightID[3] = '0' + (rand() % 10);
     emergencyFlight.flightID[4] = '\0';
     
     strcpy(emergencyFlight.origin, "UNKNOWN");
     strcpy(emergencyFlight.destination, "THIS AIRPORT");
     strcpy(emergencyFlight.aircraftType, "UNKNOWN");
     
     Time now = getCurrentTime();
     emergencyFlight.departureTime = now;
     emergencyFlight.arrivalTime = addMinutesToTime(now, 5); 
     emergencyFlight.status = EMERGENCY_STATUS;
     emergencyFlight.runwayAssigned = -1;
     emergencyFlight.crewAssigned = -1;
     emergencyFlight.delayMinutes = 0;
     emergencyFlight.isCargo = false;
     
     flights[flightCount] = emergencyFlight;
     flightCount++;
     
     char msg[STR_LEN*2];
     sprintf(msg, "EMERGENCY: Flight %s incoming!", emergencyFlight.flightID);
     addNotification(msg, false, true);
     

     assignRunways();
     scheduleCrew();
 }
 
 void handleFlightCancellation() {
     if (flightCount == 0) {
         addNotification("No flights to cancel", false, true);
         return;
     }
 
     int flightIndex = rand() % flightCount;
     flights[flightIndex].status = CANCELLED;
     
     char msg[STR_LEN*2];
     sprintf(msg, "Flight %s has been cancelled", flights[flightIndex].flightID);
     addNotification(msg, false, true);
 }
 
 void rescheduleFlights() {
     if (flightCount == 0) {
         addNotification("No flights to reschedule", false, true);
         return;
     }
 
     for (int i = 0; i < flightCount; i++) {
         if (flights[i].status != CANCELLED) {
             flights[i].runwayAssigned = -1;
             flights[i].crewAssigned = -1;
             flights[i].status = SCHEDULED;
             flights[i].delayMinutes = 0;
         }
     }
     
     for (int i = 0; i < runwayCount; i++) {
         runways[i].isAvailable = true;
         runways[i].nextAvailableTime.hour = 0;
         runways[i].nextAvailableTime.minute = 0;
     }
     
     for (int i = 0; i < crewCount; i++) {
         crews[i].isAvailable = true;
         crews[i].dutyMinutesToday = 0;
         crews[i].lastFlightEnd.hour = 0;
         crews[i].lastFlightEnd.minute = 0;
     }
     
     assignRunways();
     scheduleCrew();
     
     addNotification("All flights have been rescheduled", false, false);
 }
 
 
 void on_login_clicked(GtkWidget *widget, gpointer data) {
    const char *username = gtk_entry_get_text(GTK_ENTRY(username_entry));
    const char *password = gtk_entry_get_text(GTK_ENTRY(password_entry));

    if (strlen(username) == 0 || strlen(password) == 0) {
        gtk_label_set_text(GTK_LABEL(notification_label), "Please enter both username and password");
        return;
    }

    User authenticatedUser;
    if (authenticateUser(username, password, &authenticatedUser)) {
        currentUser = authenticatedUser;
        
        printf("Login successful for user: %s\n", currentUser.username);
        
        gtk_entry_set_text(GTK_ENTRY(password_entry), "");
        
        create_main_menu();
        
        gtk_widget_show_all(window);
        
        gtk_stack_set_visible_child_name(GTK_STACK(stack), "main_menu");
        
        char welcome_msg[256];
        sprintf(welcome_msg, "Welcome, %s (%s)", 
                currentUser.username,
                currentUser.role == ADMIN ? "Admin" : 
                currentUser.role == FLIGHT_SCHEDULER ? "Flight Scheduler" :
                currentUser.role == CREW_SCHEDULER ? "Crew Scheduler" : "Viewer");
        gtk_label_set_text(GTK_LABEL(notification_label), welcome_msg);
    } else {
        gtk_label_set_text(GTK_LABEL(notification_label), "Invalid username or password");
    }
}

 
 void on_back_clicked(GtkWidget *widget, gpointer data) {
     gtk_stack_set_visible_child_name(GTK_STACK(stack), "main_menu");
 }
 
 void on_logout_clicked(GtkWidget *widget, gpointer data) {
     gtk_stack_set_visible_child_name(GTK_STACK(stack), "login");
     gtk_entry_set_text(GTK_ENTRY(username_entry), "");
     gtk_entry_set_text(GTK_ENTRY(password_entry), "");
     gtk_label_set_text(GTK_LABEL(notification_label), "Logged out successfully");
 }
 
 void on_add_flight_clicked(GtkWidget *widget, gpointer data) {
     addFlight();
     update_flight_list();
     update_schedule_list();
     gtk_label_set_text(GTK_LABEL(notification_label), "Flight added successfully");
 }
 
 void on_modify_flight_clicked(GtkWidget *widget, gpointer data) {
     modifyFlight();
     update_flight_list();
     update_schedule_list();
     gtk_label_set_text(GTK_LABEL(notification_label), "Flight modified successfully");
 }
 
 void on_delete_flight_clicked(GtkWidget *widget, gpointer data) {
     deleteFlight();
     update_flight_list();
     update_schedule_list();
     gtk_label_set_text(GTK_LABEL(notification_label), "Flight deleted successfully");
 }
 
 void on_assign_runways_clicked(GtkWidget *widget, gpointer data) {
     assignRunways();
     update_runway_list();
     update_flight_list();
     update_schedule_list();
     gtk_label_set_text(GTK_LABEL(notification_label), "Runways assigned successfully");
 }
 
 void on_schedule_crew_clicked(GtkWidget *widget, gpointer data) {
     scheduleCrew();
     update_crew_list();
     update_flight_list();
     update_schedule_list();
     gtk_label_set_text(GTK_LABEL(notification_label), "Crew scheduled successfully");
 }
 
 void show_delay_dialog() {
     GtkWidget *dialog, *content_area, *grid, *label, *spin;
     
     dialog = gtk_dialog_new_with_buttons("Enter Delay Time",
                                        GTK_WINDOW(window),
                                        GTK_DIALOG_MODAL,
                                        "_OK",
                                        GTK_RESPONSE_ACCEPT,
                                        "_Cancel",
                                        GTK_RESPONSE_REJECT,
                                        NULL);
     
     content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
     
     grid = gtk_grid_new();
     gtk_grid_set_row_spacing(GTK_GRID(grid), 10);
     gtk_grid_set_column_spacing(GTK_GRID(grid), 10);
     gtk_container_set_border_width(GTK_CONTAINER(grid), 10);
     
     label = gtk_label_new("Delay time (minutes):");
     spin = gtk_spin_button_new_with_range(1, 300, 1);
     
     gtk_grid_attach(GTK_GRID(grid), label, 0, 0, 1, 1);
     gtk_grid_attach(GTK_GRID(grid), spin, 1, 0, 1, 1);
     
     gtk_container_add(GTK_CONTAINER(content_area), grid);
     
     g_signal_connect(dialog, "response",
                     G_CALLBACK(on_delay_dialog_response),
                     spin);
     
     gtk_widget_show_all(dialog);
 }
 
 void on_delay_dialog_response(GtkDialog *dialog, gint response_id, gpointer user_data) {
     GtkSpinButton *spin = GTK_SPIN_BUTTON(user_data);
     
     if (response_id == GTK_RESPONSE_ACCEPT) {
         int delay = gtk_spin_button_get_value_as_int(spin);
         handleWeatherDelay(delay);
         update_flight_list();
         update_runway_list();
         update_crew_list();
         update_schedule_list();
         gtk_label_set_text(GTK_LABEL(notification_label), "Weather delay simulated");
     }
     
     gtk_widget_destroy(GTK_WIDGET(dialog));
 }
 
 void on_weather_delay_clicked(GtkWidget *widget, gpointer data) {
     show_delay_dialog();
 }
 
 void on_emergency_landing_clicked(GtkWidget *widget, gpointer data) {
     handleEmergencyLanding();
     update_flight_list();
     update_runway_list();
     update_crew_list();
     update_schedule_list();
     gtk_label_set_text(GTK_LABEL(notification_label), "Emergency landing simulated");
 }
 
 void on_flight_cancellation_clicked(GtkWidget *widget, gpointer data) {
     handleFlightCancellation();
     update_flight_list();
     update_schedule_list();
     gtk_label_set_text(GTK_LABEL(notification_label), "Flight cancellation simulated");
 }
 
 void on_reschedule_clicked(GtkWidget *widget, gpointer data) {
     rescheduleFlights();
     update_flight_list();
     update_runway_list();
     update_crew_list();
     update_schedule_list();
     gtk_label_set_text(GTK_LABEL(notification_label), "All flights rescheduled");
 }
 
 void on_search_clicked(GtkWidget *widget, gpointer data) {
     const char *search_term = gtk_entry_get_text(GTK_ENTRY(search_entry));
     GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(search_results));
     gtk_text_buffer_set_text(buffer, "", -1);
 
     GtkTextIter iter;
     gtk_text_buffer_get_start_iter(buffer, &iter);
 
     if (strlen(search_term) == 0) {
         gtk_text_buffer_insert(buffer, &iter, "Please enter a search term", -1);
         return;
     }
 
     bool found = false;
     for (int i = 0; i < flightCount; i++) {
         if (strstr(flights[i].flightID, search_term) != NULL) {
             char line[512];
             const char* statusStr;
             switch(flights[i].status) {
                 case SCHEDULED: statusStr = "Scheduled"; break;
                 case DELAYED: statusStr = "Delayed"; break;
                 case CANCELLED: statusStr = "Cancelled"; break;
                 case EMERGENCY_STATUS: statusStr = "Emergency"; break;
                 default: statusStr = "Unknown";
             }
             
             sprintf(line, "Flight %s: %s to %s\n  Departure: %02d:%02d (%s)\n  Arrival: %02d:%02d\n\n",
                    flights[i].flightID,
                    flights[i].origin,
                    flights[i].destination,
                    flights[i].departureTime.hour,
                    flights[i].departureTime.minute,
                    statusStr,
                    flights[i].arrivalTime.hour,
                    flights[i].arrivalTime.minute);
             gtk_text_buffer_insert(buffer, &iter, line, -1);
             found = true;
         }
     }
 
     if (!found) {
         gtk_text_buffer_insert(buffer, &iter, "No flights found matching the search term", -1);
     }
 }
 
 void on_generate_flights_report_clicked(GtkWidget *widget, gpointer data) {
     GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(report_text));
     gtk_text_buffer_set_text(buffer, "", -1);
 
     GtkTextIter iter;
     gtk_text_buffer_get_start_iter(buffer, &iter);
 
     char line[512];
     sprintf(line, "FLIGHT REPORT\n=============\n\n");
     gtk_text_buffer_insert(buffer, &iter, line, -1);
 
     sprintf(line, "Total flights: %d\n", flightCount);
     gtk_text_buffer_insert(buffer, &iter, line, -1);
 
     int scheduled = 0, delayed = 0, cancelled = 0, emergency = 0;
     for (int i = 0; i < flightCount; i++) {
         switch(flights[i].status) {
             case SCHEDULED: scheduled++; break;
             case DELAYED: delayed++; break;
             case CANCELLED: cancelled++; break;
             case EMERGENCY_STATUS: emergency++; break;
         }
     }
 
     sprintf(line, "Scheduled: %d\nDelayed: %d\nCancelled: %d\nEmergency: %d\n\n",
            scheduled, delayed, cancelled, emergency);
     gtk_text_buffer_insert(buffer, &iter, line, -1);
     gtk_text_buffer_insert(buffer, &iter, "Flight Details:\n", -1);
     for (int i = 0; i < flightCount; i++) {
         const char* statusStr;
         switch(flights[i].status) {
             case SCHEDULED: statusStr = "Scheduled"; break;
             case DELAYED: statusStr = "Delayed"; break;
             case CANCELLED: statusStr = "Cancelled"; break;
             case EMERGENCY_STATUS: statusStr = "Emergency"; break;
             default: statusStr = "Unknown";
         }
         
         sprintf(line, "%s: %s to %s, %02d:%02d (%s)\n",
                flights[i].flightID,
                flights[i].origin,
                flights[i].destination,
                flights[i].departureTime.hour,
                flights[i].departureTime.minute,
                statusStr);
         gtk_text_buffer_insert(buffer, &iter, line, -1);
     }
 
     gtk_label_set_text(GTK_LABEL(notification_label), "Flight report generated");
 }
 
 void on_generate_runways_report_clicked(GtkWidget *widget, gpointer data) {
     GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(report_text));
     gtk_text_buffer_set_text(buffer, "", -1);
 
     GtkTextIter iter;
     gtk_text_buffer_get_start_iter(buffer, &iter);
 
     char line[512];
     sprintf(line, "RUNWAY UTILIZATION REPORT\n========================\n\n");
     gtk_text_buffer_insert(buffer, &iter, line, -1);
 
     for (int i = 0; i < runwayCount; i++) {
         const char* typeStr;
         switch(runways[i].type) {
             case ALL_FLIGHTS: typeStr = "All Flights"; break;
             case INTERNATIONAL_ONLY: typeStr = "International Only"; break;
             case CARGO_ONLY: typeStr = "Cargo Only"; break;
             default: typeStr = "Unknown";
         }
 
         sprintf(line, "Runway %d (%s): %s\n", 
                runways[i].id, typeStr,
                runways[i].isAvailable ? "Available" : "In Use");
         gtk_text_buffer_insert(buffer, &iter, line, -1);
 
         if (!runways[i].isAvailable) {
             sprintf(line, "  Next available at %02d:%02d\n\n", 
                    runways[i].nextAvailableTime.hour, 
                    runways[i].nextAvailableTime.minute);
             gtk_text_buffer_insert(buffer, &iter, line, -1);
         } else {
             gtk_text_buffer_insert(buffer, &iter, "\n", -1);
         }
     }
 
     gtk_label_set_text(GTK_LABEL(notification_label), "Runway report generated");
 }
 
 void on_generate_crew_report_clicked(GtkWidget *widget, gpointer data) {
     GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(report_text));
     gtk_text_buffer_set_text(buffer, "", -1);
 
     GtkTextIter iter;
     gtk_text_buffer_get_start_iter(buffer, &iter);
 
     char line[512];
     sprintf(line, "CREW STATUS REPORT\n==================\n\n");
     gtk_text_buffer_insert(buffer, &iter, line, -1);
 
     int available = 0, on_duty = 0;
     for (int i = 0; i < crewCount; i++) {
         if (crews[i].isAvailable) {
             available++;
         } else {
             on_duty++;
         }
     }
 
     sprintf(line, "Available crew: %d\nOn duty: %d\n\n", available, on_duty);
     gtk_text_buffer_insert(buffer, &iter, line, -1);
 
     sprintf(line, "Crew approaching duty limits:\n");
     gtk_text_buffer_insert(buffer, &iter, line, -1);
 
     bool any_approaching = false;
     for (int i = 0; i < crewCount; i++) {
         if (crews[i].dutyMinutesToday > MAX_DUTY_TIME * 0.8) {
             sprintf(line, "%s: %d/%d minutes\n", 
                    crews[i].name, 
                    crews[i].dutyMinutesToday, 
                    MAX_DUTY_TIME);
             gtk_text_buffer_insert(buffer, &iter, line, -1);
             any_approaching = true;
         }
     }
 
     if (!any_approaching) {
         gtk_text_buffer_insert(buffer, &iter, "None\n", -1);
     }
 
     gtk_label_set_text(GTK_LABEL(notification_label), "Crew report generated");
 }
 
 void on_flight_selected(GtkTreeSelection *selection, gpointer data) {
     GtkTreeModel *model;
     GtkTreeIter iter;
 
     if (gtk_tree_selection_get_selected(selection, &model, &iter)) {
         Flight *flight;
         gtk_tree_model_get(model, &iter, 1, &flight, -1);
         selectedFlightIndex = -1;
         for (int i = 0; i < flightCount; i++) {
             if (strcmp(flights[i].flightID, flight->flightID) == 0) {
                 selectedFlightIndex = i;
                 break;
             }
         }
 
         GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(flight_details_text));
         gtk_text_buffer_set_text(buffer, "", -1);
 
         GtkTextIter text_iter;
         gtk_text_buffer_get_start_iter(buffer, &text_iter);
 
         char line[256];
         sprintf(line, "Flight ID: %s\n", flight->flightID);
         gtk_text_buffer_insert(buffer, &text_iter, line, -1);
 
         sprintf(line, "Route: %s to %s\n", flight->origin, flight->destination);
         gtk_text_buffer_insert(buffer, &text_iter, line, -1);
 
         sprintf(line, "Aircraft: %s\n", flight->aircraftType);
         gtk_text_buffer_insert(buffer, &text_iter, line, -1);
 
         sprintf(line, "Departure: %02d:%02d", flight->departureTime.hour, flight->departureTime.minute);
         if (flight->delayMinutes > 0) {
             sprintf(line + strlen(line), " (Delayed by %d minutes)", flight->delayMinutes);
         }
         strcat(line, "\n");
         gtk_text_buffer_insert(buffer, &text_iter, line, -1);
 
         sprintf(line, "Arrival: %02d:%02d\n", flight->arrivalTime.hour, flight->arrivalTime.minute);
         gtk_text_buffer_insert(buffer, &text_iter, line, -1);
 
         const char* priorityStr;
         switch (flight->priority) {
             case EMERGENCY: priorityStr = "Emergency"; break;
             case INTERNATIONAL: priorityStr = "International"; break;
             case DOMESTIC: priorityStr = "Domestic"; break;
             default: priorityStr = "Unknown";
         }
         sprintf(line, "Priority: %s\n", priorityStr);
         gtk_text_buffer_insert(buffer, &text_iter, line, -1);
 
         const char* statusStr;
         char delayedStr[20];
         switch (flight->status) {
             case SCHEDULED: statusStr = "Scheduled"; break;
             case DELAYED: 
                 sprintf(delayedStr, "Delayed (%d min)", flight->delayMinutes);
                 statusStr = delayedStr;
                 break;
             case CANCELLED: statusStr = "Cancelled"; break;
             case EMERGENCY_STATUS: statusStr = "Emergency"; break;
             default: statusStr = "Unknown";
         }
         sprintf(line, "Status: %s\n", statusStr);
         gtk_text_buffer_insert(buffer, &text_iter, line, -1);
 
         sprintf(line, "Runway: %d\n", flight->runwayAssigned);
         gtk_text_buffer_insert(buffer, &text_iter, line, -1);
 
         if (flight->crewAssigned == -1) {
             sprintf(line, "Crew: Not assigned\n");
         } else {
             sprintf(line, "Crew: %s\n", crews[flight->crewAssigned].name);
         }
         gtk_text_buffer_insert(buffer, &text_iter, line, -1);
     }
 }
 
 
 void update_flight_list() {
     GtkListStore *store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_POINTER);
     GtkTreeIter iter;
 
     for (int i = 0; i < flightCount; i++) {
         gtk_list_store_append(store, &iter);
         gtk_list_store_set(store, &iter, 
                           0, flights[i].flightID,
                           1, &flights[i],
                           -1);
     }
 
     gtk_tree_view_set_model(GTK_TREE_VIEW(flight_list), GTK_TREE_MODEL(store));
     g_object_unref(store);
 }
 
 void update_runway_list() {
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(runway_list));
    gtk_text_buffer_set_text(buffer, "", -1);

    GtkTextIter iter;
    gtk_text_buffer_get_start_iter(buffer, &iter);

    for (int i = 0; i < runwayCount; i++) {
        char line[256];
        const char* typeStr;
        switch(runways[i].type) {
            case ALL_FLIGHTS: typeStr = "All Flights"; break;
            case INTERNATIONAL_ONLY: typeStr = "International Only"; break;
            case CARGO_ONLY: typeStr = "Cargo Only"; break;
            default: typeStr = "Unknown";
        }
        
        sprintf(line, "Runway %d: %s - %s\n", 
               runways[i].id, typeStr, 
               runways[i].isAvailable ? "Available" : "In Use");
        gtk_text_buffer_insert(buffer, &iter, line, -1);

        if (!runways[i].isAvailable) {
            sprintf(line, "  Next available at %02d:%02d\n\n", 
                   runways[i].nextAvailableTime.hour, 
                   runways[i].nextAvailableTime.minute);
            gtk_text_buffer_insert(buffer, &iter, line, -1);
        } else {
            gtk_text_buffer_insert(buffer, &iter, "\n", -1);
        }
    }
}
 
 void update_crew_list() {
     GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(crew_list));
     gtk_text_buffer_set_text(buffer, "", -1);
 
     GtkTextIter iter;
     gtk_text_buffer_get_start_iter(buffer, &iter);
 
     for (int i = 0; i < crewCount; i++) {
         char line[512];
         sprintf(line, "%s - %s (%d/%d mins duty)\n", 
                crews[i].name, 
                crews[i].isAvailable ? "Available" : "On Duty",
                crews[i].dutyMinutesToday, MAX_DUTY_TIME);
         gtk_text_buffer_insert(buffer, &iter, line, -1);
 
         sprintf(line, "  Qualifications: %s\n\n", crews[i].qualifications);
         gtk_text_buffer_insert(buffer, &iter, line, -1);
     }
 }
 
 void update_schedule_list() {
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(schedule_list));
    gtk_text_buffer_set_text(buffer, "", -1);

    GtkTextTag *mono_tag = gtk_text_buffer_create_tag(buffer, "monospace",
                                                    "family", "Monospace",
                                                    NULL);

    GtkTextIter iter;
    gtk_text_buffer_get_start_iter(buffer, &iter);

    gtk_text_buffer_insert_with_tags(buffer, &iter, 
        "Flight ID  Departure  Arrival   Route               Status         Runway  Crew\n", 
        -1, mono_tag, NULL);
    gtk_text_buffer_insert_with_tags(buffer, &iter, 
        "--------  ---------  -------   -----               ------         ------  ----\n", 
        -1, mono_tag, NULL);

    for (int i = 0; i < flightCount; i++) {
        char line[256];
        const char* statusStr;
        switch(flights[i].status) {
            case SCHEDULED: statusStr = "Scheduled"; break;
            case DELAYED: statusStr = g_strdup_printf("Delayed (%d)", flights[i].delayMinutes); break;
            case CANCELLED: statusStr = "Cancelled"; break;
            case EMERGENCY_STATUS: statusStr = "Emergency"; break;
            default: statusStr = "Unknown";
        }

        const char* runwayStr = (flights[i].runwayAssigned == -1) ? "None" : 
                               g_strdup_printf("Rwy %d", flights[i].runwayAssigned);
        const char* crewName = (flights[i].crewAssigned == -1) ? "None" : crews[flights[i].crewAssigned].name;

        snprintf(line, sizeof(line), 
            "%-8s  %02d:%02d     %02d:%02d   %-5s to %-5s      %-14s %-7s %s\n",
            flights[i].flightID,
            flights[i].departureTime.hour, flights[i].departureTime.minute,
            flights[i].arrivalTime.hour, flights[i].arrivalTime.minute,
            flights[i].origin, flights[i].destination,
            statusStr,
            runwayStr,
            crewName);

        gtk_text_buffer_insert_with_tags(buffer, &iter, line, -1, mono_tag, NULL);
    }
}

void clearRunwayAssignments() {
    for (int i = 0; i < flightCount; i++) {
        flights[i].runwayAssigned = -1;
    }
    
    for (int i = 0; i < runwayCount; i++) {
        runways[i].isAvailable = true;
        runways[i].nextAvailableTime.hour = 0;
        runways[i].nextAvailableTime.minute = 0;
    }
    
    addNotification("All runway assignments cleared", false, false);
    update_runway_list();
    update_flight_list();
    update_schedule_list();
}

void clearCrewAssignments() {
    for (int i = 0; i < flightCount; i++) {
        flights[i].crewAssigned = -1;
    }
    
    for (int i = 0; i < crewCount; i++) {
        crews[i].isAvailable = true;
        crews[i].dutyMinutesToday = 0;
        crews[i].lastFlightEnd.hour = 0;
        crews[i].lastFlightEnd.minute = 0;
    }
    
    addNotification("All crew assignments cleared", false, false);
    update_crew_list();
    update_flight_list();
    update_schedule_list();
}

void on_clear_runways_clicked(GtkWidget *widget, gpointer data) {
    clearRunwayAssignments();
    gtk_label_set_text(GTK_LABEL(notification_label), "Runway assignments cleared");
}

void on_clear_crew_clicked(GtkWidget *widget, gpointer data) {
    clearCrewAssignments();
    gtk_label_set_text(GTK_LABEL(notification_label), "Crew assignments cleared");
}
 
 void show_notification(const char *message) {
     gtk_label_set_text(GTK_LABEL(notification_label), message);
 }
 
 
 void create_login_screen() {
    login_grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(login_grid), 10);
    gtk_grid_set_column_spacing(GTK_GRID(login_grid), 10);
    gtk_container_set_border_width(GTK_CONTAINER(login_grid), 20);
    gtk_widget_set_halign(login_grid, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(login_grid, GTK_ALIGN_CENTER);

    GtkWidget *title = gtk_label_new("Flight Management System");
    gtk_widget_set_halign(title, GTK_ALIGN_CENTER);
    
    GtkCssProvider *provider = gtk_css_provider_new();
    gtk_css_provider_load_from_data(provider, 
        "label.title { font: 20px Sans; }", -1, NULL);
    GtkStyleContext *context = gtk_widget_get_style_context(title);
    gtk_style_context_add_provider(context, 
        GTK_STYLE_PROVIDER(provider), 
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    gtk_style_context_add_class(context, "title");
    
    gtk_grid_attach(GTK_GRID(login_grid), title, 0, 0, 2, 1);

    GtkWidget *username_label = gtk_label_new("Username:");
    gtk_widget_set_halign(username_label, GTK_ALIGN_END);
    gtk_grid_attach(GTK_GRID(login_grid), username_label, 0, 1, 1, 1);

    username_entry = gtk_entry_new();
    gtk_grid_attach(GTK_GRID(login_grid), username_entry, 1, 1, 1, 1);

    GtkWidget *password_label = gtk_label_new("Password:");
    gtk_widget_set_halign(password_label, GTK_ALIGN_END);
    gtk_grid_attach(GTK_GRID(login_grid), password_label, 0, 2, 1, 1);

    password_entry = gtk_entry_new();
    gtk_entry_set_visibility(GTK_ENTRY(password_entry), FALSE);
    gtk_grid_attach(GTK_GRID(login_grid), password_entry, 1, 2, 1, 1);

    GtkWidget *login_button = gtk_button_new_with_label("Login");
    g_signal_connect(login_button, "clicked", G_CALLBACK(on_login_clicked), NULL);
    gtk_grid_attach(GTK_GRID(login_grid), login_button, 0, 3, 2, 1);

    
gtk_widget_set_halign(notification_label, GTK_ALIGN_CENTER);
gtk_grid_attach(GTK_GRID(login_grid), notification_label, 0, 4, 2, 1);

    gtk_stack_add_named(GTK_STACK(stack), login_grid, "login");
}
 
void create_main_menu() {

    GtkWidget *existing_menu = gtk_stack_get_child_by_name(GTK_STACK(stack), "main_menu");
    if (existing_menu) {
        gtk_container_remove(GTK_CONTAINER(stack), existing_menu);
    }

    main_menu_grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(main_menu_grid), 10);
    gtk_grid_set_column_spacing(GTK_GRID(main_menu_grid), 10);
    gtk_container_set_border_width(GTK_CONTAINER(main_menu_grid), 20);
    gtk_widget_set_halign(main_menu_grid, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(main_menu_grid, GTK_ALIGN_CENTER);

    GtkWidget *header = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_widget_set_hexpand(header, TRUE);

    char welcome_text[256];
    sprintf(welcome_text, "Welcome, %s (%s)", 
            currentUser.username,
            currentUser.role == ADMIN ? "Admin" : 
            currentUser.role == FLIGHT_SCHEDULER ? "Flight Scheduler" :
            currentUser.role == CREW_SCHEDULER ? "Crew Scheduler" : "Viewer");
    GtkWidget *welcome_label = gtk_label_new(welcome_text);
    gtk_box_pack_start(GTK_BOX(header), welcome_label, FALSE, FALSE, 0);

    Time now = getCurrentTime();
    char time_str[10];
    sprintf(time_str, "%02d:%02d", now.hour, now.minute);
    GtkWidget *time_label = gtk_label_new(time_str);
    gtk_widget_set_halign(time_label, GTK_ALIGN_END);
    gtk_box_pack_end(GTK_BOX(header), time_label, FALSE, FALSE, 0);

    gtk_grid_attach(GTK_GRID(main_menu_grid), header, 0, 0, 2, 1);

    int row = 1;
    
    if (currentUser.role == ADMIN || currentUser.role == FLIGHT_SCHEDULER) {
        GtkWidget *flight_button = gtk_button_new_with_label("Flight Management");
        g_signal_connect(flight_button, "clicked", G_CALLBACK(switch_to_screen), "flight_management");
        gtk_grid_attach(GTK_GRID(main_menu_grid), flight_button, 0, row, 1, 1);
    }

    if (currentUser.role == ADMIN || currentUser.role == CREW_SCHEDULER) {
        GtkWidget *runway_button = gtk_button_new_with_label("Runway Scheduling");
        g_signal_connect(runway_button, "clicked", G_CALLBACK(switch_to_screen), "runway_management");
        gtk_grid_attach(GTK_GRID(main_menu_grid), runway_button, 
                       (currentUser.role == ADMIN || currentUser.role == FLIGHT_SCHEDULER) ? 1 : 0, 
                       row, 1, 1);
    }

    if ((currentUser.role == ADMIN || currentUser.role == FLIGHT_SCHEDULER) || 
        (currentUser.role == ADMIN || currentUser.role == CREW_SCHEDULER)) {
        row++;
    }

    if (currentUser.role == ADMIN || currentUser.role == CREW_SCHEDULER) {
        GtkWidget *crew_button = gtk_button_new_with_label("Crew Scheduling");
        g_signal_connect(crew_button, "clicked", G_CALLBACK(switch_to_screen), "crew_management");
        gtk_grid_attach(GTK_GRID(main_menu_grid), crew_button, 0, row, 1, 1);
    }

    if (currentUser.role == ADMIN) {
        GtkWidget *events_button = gtk_button_new_with_label("Real-Time Events");
        g_signal_connect(events_button, "clicked", G_CALLBACK(switch_to_screen), "realtime_events");
        gtk_grid_attach(GTK_GRID(main_menu_grid), events_button, 
                       (currentUser.role == ADMIN || currentUser.role == CREW_SCHEDULER) ? 1 : 0, 
                       row, 1, 1);
    }

    if (currentUser.role == ADMIN || currentUser.role == CREW_SCHEDULER) {
        row++;
    }

    GtkWidget *schedule_button = gtk_button_new_with_label("Flight Schedule");
    g_signal_connect(schedule_button, "clicked", G_CALLBACK(switch_to_screen), "schedule");
    gtk_grid_attach(GTK_GRID(main_menu_grid), schedule_button, 0, row, 1, 1);

    GtkWidget *search_button = gtk_button_new_with_label("Search Flight");
    g_signal_connect(search_button, "clicked", G_CALLBACK(switch_to_screen), "search_flight");
    gtk_grid_attach(GTK_GRID(main_menu_grid), search_button, 1, row, 1, 1);
    row++;

    if (currentUser.role == ADMIN || currentUser.role == VIEWER) {
        GtkWidget *reports_button = gtk_button_new_with_label("Reports");
        g_signal_connect(reports_button, "clicked", G_CALLBACK(switch_to_screen), "reports");
        gtk_grid_attach(GTK_GRID(main_menu_grid), reports_button, 0, row, 2, 1);
        row++;
    }

    GtkWidget *logout_button = gtk_button_new_with_label("Logout");
    g_signal_connect(logout_button, "clicked", G_CALLBACK(on_logout_clicked), NULL);
    gtk_grid_attach(GTK_GRID(main_menu_grid), logout_button, 0, row, 2, 1);

    gtk_stack_add_named(GTK_STACK(stack), main_menu_grid, "main_menu");
    gtk_stack_set_visible_child_name(GTK_STACK(stack), "main_menu");
    gtk_widget_show_all(main_menu_grid);
}
 
 void create_flight_management() {
    flight_management_grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(flight_management_grid), 10);
    gtk_grid_set_column_spacing(GTK_GRID(flight_management_grid), 10);
    gtk_container_set_border_width(GTK_CONTAINER(flight_management_grid), 10);

    GtkWidget *header = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_widget_set_hexpand(header, TRUE);

    GtkWidget *title = gtk_label_new("Flight Management");
    gtk_box_pack_start(GTK_BOX(header), title, FALSE, FALSE, 0);

    GtkWidget *back_button = gtk_button_new_with_label("Back");
    g_signal_connect(back_button, "clicked", G_CALLBACK(on_back_clicked), NULL);
    gtk_box_pack_end(GTK_BOX(header), back_button, FALSE, FALSE, 0);

    gtk_grid_attach(GTK_GRID(flight_management_grid), header, 0, 0, 2, 1);

    GtkWidget *scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_widget_set_hexpand(scrolled_window, TRUE);
    gtk_widget_set_vexpand(scrolled_window, TRUE);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window),
                                 GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

    flight_list = gtk_tree_view_new();
    GtkCellRenderer *flight_renderer = gtk_cell_renderer_text_new();
    GtkTreeViewColumn *column = gtk_tree_view_column_new_with_attributes("Flight ID",
                                                                       flight_renderer,
                                                                       "text", 0,
                                                                       NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(flight_list), column);

    GtkListStore *flight_store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_POINTER);
    gtk_tree_view_set_model(GTK_TREE_VIEW(flight_list), GTK_TREE_MODEL(flight_store));
    g_object_unref(flight_store);

    GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(flight_list));
    g_signal_connect(selection, "changed", G_CALLBACK(on_flight_selected), NULL);

    gtk_container_add(GTK_CONTAINER(scrolled_window), flight_list);
    gtk_grid_attach(GTK_GRID(flight_management_grid), scrolled_window, 0, 1, 1, 1);

    GtkWidget *details_scrolled = gtk_scrolled_window_new(NULL, NULL);
    gtk_widget_set_hexpand(details_scrolled, TRUE);
    gtk_widget_set_vexpand(details_scrolled, TRUE);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(details_scrolled),
                                 GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

    flight_details_text = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(flight_details_text), FALSE);
    gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(flight_details_text), FALSE);
    gtk_container_add(GTK_CONTAINER(details_scrolled), flight_details_text);
    gtk_grid_attach(GTK_GRID(flight_management_grid), details_scrolled, 1, 1, 1, 1);

    GtkWidget *form_grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(form_grid), 5);
    gtk_grid_set_column_spacing(GTK_GRID(form_grid), 5);
    gtk_grid_attach(GTK_GRID(flight_management_grid), form_grid, 0, 2, 2, 1);

    GtkWidget *flight_id_label = gtk_label_new("Flight ID:");
    gtk_widget_set_halign(flight_id_label, GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(form_grid), flight_id_label, 0, 0, 1, 1);

    flight_id_entry = gtk_entry_new();
    gtk_grid_attach(GTK_GRID(form_grid), flight_id_entry, 1, 0, 1, 1);

    GtkWidget *origin_label = gtk_label_new("Origin:");
    gtk_widget_set_halign(origin_label, GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(form_grid), origin_label, 0, 1, 1, 1);

    origin_entry = gtk_entry_new();
    gtk_grid_attach(GTK_GRID(form_grid), origin_entry, 1, 1, 1, 1);

    GtkWidget *destination_label = gtk_label_new("Destination:");
    gtk_widget_set_halign(destination_label, GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(form_grid), destination_label, 0, 2, 1, 1);

    destination_entry = gtk_entry_new();
    gtk_grid_attach(GTK_GRID(form_grid), destination_entry, 1, 2, 1, 1);

    GtkWidget *aircraft_label = gtk_label_new("Aircraft Type:");
    gtk_widget_set_halign(aircraft_label, GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(form_grid), aircraft_label, 0, 3, 1, 1);

    GtkListStore *aircraft_store = gtk_list_store_new(1, G_TYPE_STRING);
    GtkTreeIter aircraft_iter;
    const char *aircraft_types[] = {"Boeing737", "AirbusA320", "Boeing787", 
                                   "AirbusA350", "Embraer190", "Boeing777", 
                                   "AirbusA380", NULL};

    for (int i = 0; aircraft_types[i] != NULL; i++) {
        gtk_list_store_append(aircraft_store, &aircraft_iter);
        gtk_list_store_set(aircraft_store, &aircraft_iter, 0, aircraft_types[i], -1);
    }

    aircraft_entry = gtk_combo_box_new_with_model(GTK_TREE_MODEL(aircraft_store));
    GtkCellRenderer *aircraft_renderer = gtk_cell_renderer_text_new();
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(aircraft_entry), aircraft_renderer, TRUE);
    gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(aircraft_entry), aircraft_renderer, 
                                 "text", 0, NULL);
    gtk_combo_box_set_active(GTK_COMBO_BOX(aircraft_entry), 0);
    gtk_grid_attach(GTK_GRID(form_grid), aircraft_entry, 1, 3, 1, 1);

    GtkWidget *dep_time_label = gtk_label_new("Departure Time (HH:MM):");
    gtk_widget_set_halign(dep_time_label, GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(form_grid), dep_time_label, 0, 4, 1, 1);

    GtkWidget *time_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    dep_hour_spin = gtk_spin_button_new_with_range(0, 23, 1);
    dep_min_spin = gtk_spin_button_new_with_range(0, 59, 1);
    gtk_box_pack_start(GTK_BOX(time_box), dep_hour_spin, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(time_box), gtk_label_new(":"), FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(time_box), dep_min_spin, TRUE, TRUE, 0);
    gtk_grid_attach(GTK_GRID(form_grid), time_box, 1, 4, 1, 1);

    GtkWidget *duration_label = gtk_label_new("Duration (minutes):");
    gtk_widget_set_halign(duration_label, GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(form_grid), duration_label, 0, 5, 1, 1);

    duration_spin = gtk_spin_button_new_with_range(1, 1440, 1);
    gtk_grid_attach(GTK_GRID(form_grid), duration_spin, 1, 5, 1, 1);

    GtkWidget *priority_label = gtk_label_new("Priority:");
    gtk_widget_set_halign(priority_label, GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(form_grid), priority_label, 0, 6, 1, 1);

    GtkListStore *priority_store = gtk_list_store_new(1, G_TYPE_STRING);
    GtkTreeIter iter;

    gtk_list_store_append(priority_store, &iter);
    gtk_list_store_set(priority_store, &iter, 0, "Emergency", -1);

    gtk_list_store_append(priority_store, &iter);
    gtk_list_store_set(priority_store, &iter, 0, "International", -1);

    gtk_list_store_append(priority_store, &iter);
    gtk_list_store_set(priority_store, &iter, 0, "Domestic", -1);

    priority_combo_text = gtk_combo_box_new_with_model(GTK_TREE_MODEL(priority_store));
    GtkCellRenderer *combo_renderer = gtk_cell_renderer_text_new();
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(priority_combo_text), combo_renderer, TRUE);
    gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(priority_combo_text), combo_renderer, 
                                 "text", 0, NULL);
    gtk_combo_box_set_active(GTK_COMBO_BOX(priority_combo_text), 2);
    gtk_grid_attach(GTK_GRID(form_grid), priority_combo_text, 1, 6, 1, 1);

    cargo_check = gtk_check_button_new_with_label("Cargo Flight");
    gtk_grid_attach(GTK_GRID(form_grid), cargo_check, 1, 7, 1, 1);

    GtkWidget *delay_label = gtk_label_new("Delay (minutes):");
    gtk_widget_set_halign(delay_label, GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(form_grid), delay_label, 0, 8, 1, 1);

    delay_spin = gtk_spin_button_new_with_range(0, 300, 1);
    gtk_grid_attach(GTK_GRID(form_grid), delay_spin, 1, 8, 1, 1);

GtkWidget *button_box = gtk_button_box_new(GTK_ORIENTATION_HORIZONTAL);
gtk_button_box_set_layout(GTK_BUTTON_BOX(button_box), GTK_BUTTONBOX_START);
gtk_grid_attach(GTK_GRID(flight_management_grid), button_box, 0, 3, 2, 1);

GtkWidget *add_button = gtk_button_new_with_label("Add Flight");
g_signal_connect(add_button, "clicked", G_CALLBACK(on_add_flight_clicked), NULL);
gtk_widget_set_sensitive(add_button, currentUser.role == ADMIN || currentUser.role == FLIGHT_SCHEDULER);
gtk_container_add(GTK_CONTAINER(button_box), add_button);

GtkWidget *mod_button = gtk_button_new_with_label("Modify Flight");
g_signal_connect(mod_button, "clicked", G_CALLBACK(on_modify_flight_clicked), NULL);
gtk_widget_set_sensitive(mod_button, currentUser.role == ADMIN || currentUser.role == FLIGHT_SCHEDULER);
gtk_container_add(GTK_CONTAINER(button_box), mod_button);

GtkWidget *del_button = gtk_button_new_with_label("Delete Flight");
g_signal_connect(del_button, "clicked", G_CALLBACK(on_delete_flight_clicked), NULL);
gtk_widget_set_sensitive(del_button, currentUser.role == ADMIN || currentUser.role == FLIGHT_SCHEDULER);
gtk_container_add(GTK_CONTAINER(button_box), del_button);

    gtk_widget_set_halign(notification_label, GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(flight_management_grid), notification_label, 0, 4, 2, 1);

    gtk_stack_add_named(GTK_STACK(stack), flight_management_grid, "flight_management");
}
 void create_runway_management() {
     runway_management_grid = gtk_grid_new();
     gtk_grid_set_row_spacing(GTK_GRID(runway_management_grid), 10);
     gtk_grid_set_column_spacing(GTK_GRID(runway_management_grid), 10);
     gtk_container_set_border_width(GTK_CONTAINER(runway_management_grid), 10);
 

     GtkWidget *header = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
     gtk_widget_set_hexpand(header, TRUE);
 
     GtkWidget *title = gtk_label_new("Runway Management");
     gtk_box_pack_start(GTK_BOX(header), title, FALSE, FALSE, 0);
 
     GtkWidget *back_button = gtk_button_new_with_label("Back");
     g_signal_connect(back_button, "clicked", G_CALLBACK(on_back_clicked), NULL);
     gtk_box_pack_end(GTK_BOX(header), back_button, FALSE, FALSE, 0);
 
     gtk_grid_attach(GTK_GRID(runway_management_grid), header, 0, 0, 1, 1);
 
     GtkWidget *scrolled_window = gtk_scrolled_window_new(NULL, NULL);
     gtk_widget_set_hexpand(scrolled_window, TRUE);
     gtk_widget_set_vexpand(scrolled_window, TRUE);
     gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window),
                                   GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
 
     runway_list = gtk_text_view_new();
     gtk_text_view_set_editable(GTK_TEXT_VIEW(runway_list), FALSE);
     gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(runway_list), FALSE);
     gtk_container_add(GTK_CONTAINER(scrolled_window), runway_list);
     gtk_grid_attach(GTK_GRID(runway_management_grid), scrolled_window, 0, 1, 1, 1);

GtkWidget *assign_button = gtk_button_new_with_label("Assign Runways");
g_signal_connect(assign_button, "clicked", G_CALLBACK(on_assign_runways_clicked), NULL);
gtk_widget_set_sensitive(assign_button, currentUser.role == ADMIN || currentUser.role == CREW_SCHEDULER);
gtk_grid_attach(GTK_GRID(runway_management_grid), assign_button, 0, 2, 1, 1);
 

     notification_label = gtk_label_new("");
     gtk_widget_set_halign(notification_label, GTK_ALIGN_START);
     gtk_grid_attach(GTK_GRID(runway_management_grid), notification_label, 0, 3, 1, 1);
 
     gtk_stack_add_named(GTK_STACK(stack), runway_management_grid, "runway_management");
 }
 
 void create_crew_management() {
     crew_management_grid = gtk_grid_new();
     gtk_grid_set_row_spacing(GTK_GRID(crew_management_grid), 10);
     gtk_grid_set_column_spacing(GTK_GRID(crew_management_grid), 10);
     gtk_container_set_border_width(GTK_CONTAINER(crew_management_grid), 10);
 

     GtkWidget *header = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
     gtk_widget_set_hexpand(header, TRUE);
 
     GtkWidget *title = gtk_label_new("Crew Management");
     gtk_box_pack_start(GTK_BOX(header), title, FALSE, FALSE, 0);
 
     GtkWidget *back_button = gtk_button_new_with_label("Back");
     g_signal_connect(back_button, "clicked", G_CALLBACK(on_back_clicked), NULL);
     gtk_box_pack_end(GTK_BOX(header), back_button, FALSE, FALSE, 0);
 
     gtk_grid_attach(GTK_GRID(crew_management_grid), header, 0, 0, 1, 1);
 

     GtkWidget *scrolled_window = gtk_scrolled_window_new(NULL, NULL);
     gtk_widget_set_hexpand(scrolled_window, TRUE);
     gtk_widget_set_vexpand(scrolled_window, TRUE);
     gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window),
                                   GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
 
     crew_list = gtk_text_view_new();
     gtk_text_view_set_editable(GTK_TEXT_VIEW(crew_list), FALSE);
     gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(crew_list), FALSE);
     gtk_container_add(GTK_CONTAINER(scrolled_window), crew_list);
     gtk_grid_attach(GTK_GRID(crew_management_grid), scrolled_window, 0, 1, 1, 1);
 
GtkWidget *schedule_button = gtk_button_new_with_label("Schedule Crew");
g_signal_connect(schedule_button, "clicked", G_CALLBACK(on_schedule_crew_clicked), NULL);
gtk_widget_set_sensitive(schedule_button, currentUser.role == ADMIN || currentUser.role == CREW_SCHEDULER);
gtk_grid_attach(GTK_GRID(crew_management_grid), schedule_button, 0, 2, 1, 1);
 

     notification_label = gtk_label_new("");
     gtk_widget_set_halign(notification_label, GTK_ALIGN_START);
     gtk_grid_attach(GTK_GRID(crew_management_grid), notification_label, 0, 3, 1, 1);
 
     gtk_stack_add_named(GTK_STACK(stack), crew_management_grid, "crew_management");
 }
 
 void create_realtime_events() {
     realtime_events_grid = gtk_grid_new();
     gtk_grid_set_row_spacing(GTK_GRID(realtime_events_grid), 10);
     gtk_grid_set_column_spacing(GTK_GRID(realtime_events_grid), 10);
     gtk_container_set_border_width(GTK_CONTAINER(realtime_events_grid), 10);
 

     GtkWidget *header = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
     gtk_widget_set_hexpand(header, TRUE);
 
     GtkWidget *title = gtk_label_new("Real-Time Events");
     gtk_box_pack_start(GTK_BOX(header), title, FALSE, FALSE, 0);
 
     GtkWidget *back_button = gtk_button_new_with_label("Back");
     g_signal_connect(back_button, "clicked", G_CALLBACK(on_back_clicked), NULL);
     gtk_box_pack_end(GTK_BOX(header), back_button, FALSE, FALSE, 0);
 
     gtk_grid_attach(GTK_GRID(realtime_events_grid), header, 0, 0, 1, 1);
 

GtkWidget *weather_button = gtk_button_new_with_label("Simulate Weather Delay");
g_signal_connect(weather_button, "clicked", G_CALLBACK(on_weather_delay_clicked), NULL);
gtk_widget_set_sensitive(weather_button, currentUser.role == ADMIN);
gtk_grid_attach(GTK_GRID(realtime_events_grid), weather_button, 0, 1, 1, 1);

GtkWidget *emergency_button = gtk_button_new_with_label("Simulate Emergency Landing");
g_signal_connect(emergency_button, "clicked", G_CALLBACK(on_emergency_landing_clicked), NULL);
gtk_widget_set_sensitive(emergency_button, currentUser.role == ADMIN);
gtk_grid_attach(GTK_GRID(realtime_events_grid), emergency_button, 0, 2, 1, 1);

GtkWidget *cancel_button = gtk_button_new_with_label("Simulate Flight Cancellation");
g_signal_connect(cancel_button, "clicked", G_CALLBACK(on_flight_cancellation_clicked), NULL);
gtk_widget_set_sensitive(cancel_button, currentUser.role == ADMIN);
gtk_grid_attach(GTK_GRID(realtime_events_grid), cancel_button, 0, 3, 1, 1);

GtkWidget *reschedule_button = gtk_button_new_with_label("Reschedule All Flights");
g_signal_connect(reschedule_button, "clicked", G_CALLBACK(on_reschedule_clicked), NULL);
gtk_widget_set_sensitive(reschedule_button, currentUser.role == ADMIN);
gtk_grid_attach(GTK_GRID(realtime_events_grid), reschedule_button, 0, 4, 1, 1);

        
    GtkWidget *clear_runways_button = gtk_button_new_with_label("Clear Runway Assignments");
    g_signal_connect(clear_runways_button, "clicked", G_CALLBACK(on_clear_runways_clicked), NULL);
    gtk_grid_attach(GTK_GRID(realtime_events_grid), clear_runways_button, 0, 5, 1, 1);

    GtkWidget *clear_crew_button = gtk_button_new_with_label("Clear Crew Assignments");
    g_signal_connect(clear_crew_button, "clicked", G_CALLBACK(on_clear_crew_clicked), NULL);
    gtk_grid_attach(GTK_GRID(realtime_events_grid), clear_crew_button, 0, 6, 1, 1);


    gtk_grid_attach(GTK_GRID(realtime_events_grid), notification_label, 0, 7, 1, 1);
 
 
     notification_label = gtk_label_new("");
     gtk_widget_set_halign(notification_label, GTK_ALIGN_START);
     gtk_grid_attach(GTK_GRID(realtime_events_grid), notification_label, 0, 5, 1, 1);
    
     gtk_stack_add_named(GTK_STACK(stack), realtime_events_grid, "realtime_events");
 }
 
 void create_schedule() {
     schedule_grid = gtk_grid_new();
     gtk_grid_set_row_spacing(GTK_GRID(schedule_grid), 10);
     gtk_grid_set_column_spacing(GTK_GRID(schedule_grid), 10);
     gtk_container_set_border_width(GTK_CONTAINER(schedule_grid), 10);
 

     GtkWidget *header = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
     gtk_widget_set_hexpand(header, TRUE);
 
     GtkWidget *title = gtk_label_new("Flight Schedule");
     gtk_box_pack_start(GTK_BOX(header), title, FALSE, FALSE, 0);
 
     GtkWidget *back_button = gtk_button_new_with_label("Back");
     g_signal_connect(back_button, "clicked", G_CALLBACK(on_back_clicked), NULL);
     gtk_box_pack_end(GTK_BOX(header), back_button, FALSE, FALSE, 0);
 
     gtk_grid_attach(GTK_GRID(schedule_grid), header, 0, 0, 1, 1);
 

     GtkWidget *scrolled_window = gtk_scrolled_window_new(NULL, NULL);
     gtk_widget_set_hexpand(scrolled_window, TRUE);
     gtk_widget_set_vexpand(scrolled_window, TRUE);
     gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window),
                                   GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
 
     schedule_list = gtk_text_view_new();
     gtk_text_view_set_editable(GTK_TEXT_VIEW(schedule_list), FALSE);
     gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(schedule_list), FALSE);
     gtk_text_view_set_monospace(GTK_TEXT_VIEW(schedule_list), TRUE);
     gtk_container_add(GTK_CONTAINER(scrolled_window), schedule_list);
     gtk_grid_attach(GTK_GRID(schedule_grid), scrolled_window, 0, 1, 1, 1);
 

     notification_label = gtk_label_new("");
     gtk_widget_set_halign(notification_label, GTK_ALIGN_START);
     gtk_grid_attach(GTK_GRID(schedule_grid), notification_label, 0, 2, 1, 1);
 
     gtk_stack_add_named(GTK_STACK(stack), schedule_grid, "schedule");
 }
 
 void create_search_flight() {
     search_flight_grid = gtk_grid_new();
     gtk_grid_set_row_spacing(GTK_GRID(search_flight_grid), 10);
     gtk_grid_set_column_spacing(GTK_GRID(search_flight_grid), 10);
     gtk_container_set_border_width(GTK_CONTAINER(search_flight_grid), 10);
 

     GtkWidget *header = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
     gtk_widget_set_hexpand(header, TRUE);
 
     GtkWidget *title = gtk_label_new("Search Flight");
     gtk_box_pack_start(GTK_BOX(header), title, FALSE, FALSE, 0);
 
     GtkWidget *back_button = gtk_button_new_with_label("Back");
     g_signal_connect(back_button, "clicked", G_CALLBACK(on_back_clicked), NULL);
     gtk_box_pack_end(GTK_BOX(header), back_button, FALSE, FALSE, 0);
 
     gtk_grid_attach(GTK_GRID(search_flight_grid), header, 0, 0, 1, 1);
 

     search_entry = gtk_entry_new();
     gtk_entry_set_placeholder_text(GTK_ENTRY(search_entry), "Enter flight ID");
     gtk_grid_attach(GTK_GRID(search_flight_grid), search_entry, 0, 1, 1, 1);
 

     GtkWidget *search_button = gtk_button_new_with_label("Search");
     g_signal_connect(search_button, "clicked", G_CALLBACK(on_search_clicked), NULL);
     gtk_grid_attach(GTK_GRID(search_flight_grid), search_button, 0, 2, 1, 1);
 

     GtkWidget *scrolled_window = gtk_scrolled_window_new(NULL, NULL);
     gtk_widget_set_hexpand(scrolled_window, TRUE);
     gtk_widget_set_vexpand(scrolled_window, TRUE);
     gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window),
                                   GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
 
     search_results = gtk_text_view_new();
     gtk_text_view_set_editable(GTK_TEXT_VIEW(search_results), FALSE);
     gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(search_results), FALSE);
     gtk_container_add(GTK_CONTAINER(scrolled_window), search_results);
     gtk_grid_attach(GTK_GRID(search_flight_grid), scrolled_window, 0, 3, 1, 1);
 

     notification_label = gtk_label_new("");
     gtk_widget_set_halign(notification_label, GTK_ALIGN_START);
     gtk_grid_attach(GTK_GRID(search_flight_grid), notification_label, 0, 4, 1, 1);
 
     gtk_stack_add_named(GTK_STACK(stack), search_flight_grid, "search_flight");
 }
 
 void create_reports() {
     reports_grid = gtk_grid_new();
     gtk_grid_set_row_spacing(GTK_GRID(reports_grid), 10);
     gtk_grid_set_column_spacing(GTK_GRID(reports_grid), 10);
     gtk_container_set_border_width(GTK_CONTAINER(reports_grid), 10);


     GtkWidget *header = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
     gtk_widget_set_hexpand(header, TRUE);
 
     GtkWidget *title = gtk_label_new("Reports");
     gtk_box_pack_start(GTK_BOX(header), title, FALSE, FALSE, 0);
 
     GtkWidget *back_button = gtk_button_new_with_label("Back");
     g_signal_connect(back_button, "clicked", G_CALLBACK(on_back_clicked), NULL);
     gtk_box_pack_end(GTK_BOX(header), back_button, FALSE, FALSE, 0);
 
     gtk_grid_attach(GTK_GRID(reports_grid), header, 0, 0, 1, 1);
 

     GtkWidget *flights_report_button = gtk_button_new_with_label("Generate Flights Report");
     g_signal_connect(flights_report_button, "clicked", G_CALLBACK(on_generate_flights_report_clicked), NULL);
     gtk_grid_attach(GTK_GRID(reports_grid), flights_report_button, 0, 1, 1, 1);
 
     GtkWidget *runways_report_button = gtk_button_new_with_label("Generate Runways Report");
     g_signal_connect(runways_report_button, "clicked", G_CALLBACK(on_generate_runways_report_clicked), NULL);
     gtk_grid_attach(GTK_GRID(reports_grid), runways_report_button, 0, 2, 1, 1);
 
     GtkWidget *crew_report_button = gtk_button_new_with_label("Generate Crew Report");
     g_signal_connect(crew_report_button, "clicked", G_CALLBACK(on_generate_crew_report_clicked), NULL);
     gtk_grid_attach(GTK_GRID(reports_grid), crew_report_button, 0, 3, 1, 1);
 

     GtkWidget *scrolled_window = gtk_scrolled_window_new(NULL, NULL);
     gtk_widget_set_hexpand(scrolled_window, TRUE);
     gtk_widget_set_vexpand(scrolled_window, TRUE);
     gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window),
                                   GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
 
     report_text = gtk_text_view_new();
     gtk_text_view_set_editable(GTK_TEXT_VIEW(report_text), FALSE);
     gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(report_text), FALSE);
     gtk_container_add(GTK_CONTAINER(scrolled_window), report_text);
     gtk_grid_attach(GTK_GRID(reports_grid), scrolled_window, 0, 4, 1, 1);
 

     notification_label = gtk_label_new("");
     gtk_widget_set_halign(notification_label, GTK_ALIGN_START);
     gtk_grid_attach(GTK_GRID(reports_grid), notification_label, 0, 5, 1, 1);
 
     gtk_stack_add_named(GTK_STACK(stack), reports_grid, "reports");
 }
 
 void create_logout_screen() {
     logout_grid = gtk_grid_new();
     gtk_grid_set_row_spacing(GTK_GRID(logout_grid), 10);
     gtk_grid_set_column_spacing(GTK_GRID(logout_grid), 10);
     gtk_container_set_border_width(GTK_CONTAINER(logout_grid), 20);
     gtk_widget_set_halign(logout_grid, GTK_ALIGN_CENTER);
     gtk_widget_set_valign(logout_grid, GTK_ALIGN_CENTER);
 
     GtkWidget *label = gtk_label_new("Logging out...");
     gtk_grid_attach(GTK_GRID(logout_grid), label, 0, 0, 1, 1);
 
     gtk_stack_add_named(GTK_STACK(stack), logout_grid, "logout");
 }
 

 
 int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);


    initializeSystem();
    initializeUsers();
    loadDataFromFiles();


    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Flight Management System");
    gtk_window_set_default_size(GTK_WINDOW(window), 1000, 700);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);


    stack = gtk_stack_new();
    gtk_stack_set_transition_type(GTK_STACK(stack), GTK_STACK_TRANSITION_TYPE_SLIDE_LEFT_RIGHT);
    gtk_container_add(GTK_CONTAINER(window), stack);

    create_login_screen();
    create_flight_management();
    create_runway_management();
    create_crew_management();
    create_realtime_events();
    create_schedule();
    create_search_flight();
    create_reports();
    create_logout_screen();

    gtk_stack_set_visible_child_name(GTK_STACK(stack), "login");

    gtk_widget_show_all(window);
    gtk_main();

    saveDataToFiles();
    return 0;
}
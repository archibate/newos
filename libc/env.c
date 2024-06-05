#include <stdlib.h>
#include <string.h>

static char **_environ;

char ***__environ_location() {
    return &_environ;
}

char *getenv(const char *name)
{
	for (char **envp = environ; *envp; envp++) {
		char *s = *envp;
		while (*s && *s != '=') s++;
		if (*s == '=' && (ptrdiff_t)strlen(name) == s - *envp &&
			!memcmp(name, *envp, s - *envp))
			return s + 1;
	}
	return NULL;
}

#if 0
extern int _environ_size;
// TODO: Under construction!! setenv, putenv, unsetenv has BUG!!!!
int setenv(const char *name, const char *value, int overwrite)
{
	if (!overwrite && getenv(name))
		return 0;

	char *tmp = malloc(strlen(name) + strlen(value) + 2);
	strcpy(tmp, name);
	strcat(tmp, "=");
	strcat(tmp, value);
	return putenv(tmp);
}

static int why_no_strnstr(char * a, char * b, int n)
{
	for (int i = 0; (i < n) && (a[i]) && (b[i]); ++i) {
		if (a[i] != b[i]) {
			return 1;
		}
	}
	return 0;
}

int unsetenv(const char *name)
{
	int last_index = -1;
	int found_index = -1;
	int len = strlen(name);

	for (int i = 0; environ[i]; ++i) {
		if (found_index == -1 &&
			!memcmp(environ[i], name, len) &&
				environ[i][len] == '=') {
			found_index = i;
		}
		last_index = i;
	}

	if (found_index == -1) {
		/* not found = success */
		return 0;
	}

	if (last_index == found_index) {
		/* Was last element */
		environ[last_index] = NULL;
		return 0;
	}

	/* Was not last element, swap ordering */
	environ[found_index] = environ[last_index];
	environ[last_index] = NULL;
	return 0;
}


int putenv(char *string)
{
	char name[strlen(string) + 1];
	strcpy(name, string);
	char *c = strchr(name, '=');
	if (!c) {
		return 1;
	}
	*c = 0;

	int s = strlen(name);

	int i;
	for (i = 0; i < (_environ_size - 1) && environ[i]; ++i) {
		if (!why_no_strnstr(name, environ[i], s) && environ[i][s] == '=') {
			environ[i] = string;
			return 0;
		}
	}
	/* Not found */
	if (i == _environ_size - 1) {
		int _new_environ_size = _environ_size * 2;
		char **new_environ = malloc(sizeof(char *) * _new_environ_size);
		int j = 0;
		while (j < _new_environ_size && environ[j]) {
			new_environ[j] = environ[j];
			j++;
		}
		while (j < _new_environ_size) {
			new_environ[j] = NULL;
			j++;
		}
		_environ_size = _new_environ_size;
		environ = new_environ;
	}

	environ[i] = string;
	return 0;
}
#endif

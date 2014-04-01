#include "version.h"

void
hoedown_version(int *ver_major, int *ver_minor, int *ver_revision)
{
	*ver_major = HOEDOWN_VERSION_MAJOR;
	*ver_minor = HOEDOWN_VERSION_MINOR;
	*ver_revision = HOEDOWN_VERSION_REVISION;
}

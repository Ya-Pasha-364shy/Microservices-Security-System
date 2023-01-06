#ifndef BACKUP_SERVICE_FS_ITERATION_H
#define BACKUP_SERVICE_FS_ITERATION_H

#include <sys/stat.h>
#include <err.h>
#include <fts.h>

#include "backup-service-main.h"
#include "../../helpers/helpers-common.h"

int backup_service_fs_iteration_main(char * const * argv);

#endif // BACKUP_SERVICE_FS_ITERATION_H
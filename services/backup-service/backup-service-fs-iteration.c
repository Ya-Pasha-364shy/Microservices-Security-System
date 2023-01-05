/*
 * \author Pavel Chernov
 * \brief  Need for simple backup all files (only them)
 *         in some directory recursive or
 *         searching any file in some directory
 * 
 * \date   05.01.2023
*/
#include "../../include/services/backup-service/backup-service-fs-iteration.h"

static int ptree(char * const argv[])
{
    FTS    *ftsp;
    FTSENT *p, *chp;
    int fts_options = FTS_COMFOLLOW | FTS_LOGICAL | FTS_NOCHDIR;

    if (NULL == (ftsp = fts_open(argv, fts_options, NULL)))
    {
        warn("Can't open %s", argv);
        return BACKUP_SERVICE_ERROR_CODE;
    }

    chp = fts_children(ftsp, 0);
    if (NULL == chp)
    {
        return BACKUP_SERVICE_NORMAL_CODE;
    }
    while (NULL != (p = fts_read(ftsp)))
    {
        switch (p->fts_info)
        {
            case FTS_D:
            {
                printf("d %s\n", p->fts_path);
                break;
            }
            case FTS_F:
            {
                printf("f %s\n", p->fts_path);
                break;
            }
            default:
            {
                break;
            }
        }
    }

    fts_close(ftsp);
    return BACKUP_SERVICE_NORMAL_CODE;
}

int backup_service_fs_iteration_main(char * const argv[])
{
    int rc;
    if (BACKUP_SERVICE_NORMAL_CODE != (rc = ptree(argv + 1)))
    {
        rc = BACKUP_SERVICE_ERROR_CODE;
    }

    return rc;
}
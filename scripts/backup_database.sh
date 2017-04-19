#!/bin/bash
mysqldump -hlocalhost -uroot -proot cage | gzip > /home/cage/cagedocu/db_backups/backup-`date -I`.sql.gz

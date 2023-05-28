# Microservices-Security-System

## Автор/Autor
Pavel Chernov (K1rch)

## 1. Описание и мотивация/Description and motivation

<b>ru:</b>

Невозможно создать монолитную систему информационной безопасности,
которая будет стабильно работать, потому что падение (или отключение)
одного из модулей системы повлечёт за собой полное отключение системы
информационной безопасности.
Если такая система будет единственным источником обеспечения
информационной безопасности для некоторой компании или лично для
Вашей инфраструктуры, то несложно представить размер утечек
конфиденциальной, коммерческой или любой другой информации,
представляющей ценность для компании при отключении или выходе
из строя такой системы.

Во избежание вышеупомянутой ситуации, архитектура системы
информационной безопасности была выбрана микросервисной.
Основная мотивация к созданию микросервисной системы
информационной безопасности
заключается в достижении отказоустойчивости и распределённости.

<b>en:</b>

It is impossible to create a monolithic information security system,
which will work stably, because the fall (or shutdown) one of the
system modules will result in a complete shutdown of the system
information security.
If such a system is the only source of security
information security for a certain company or personally for
of your infrastructure, it is easy to imagine the size of leaks
of confidential, commercial or any other information of value
to the company when shutdown or failure of such a system.

To avoid the above situation, the system architecture
information security was chosen as microservice.
The main motivation for creating a microservice system
information security is to achieve fault tolerance and distribution.

### 1.1 Отказоустойчивость и распределённость

<b>ru:</b>

Отказоустойчивость можно описать таким образом, при ошибке или отказе
работы одного сервиса – на работу других сервисов это никак не повлияет.
Даже при отключении сервиса, отвечающего за коммуникацию между
микросервисами и их синхронизацию, на работу других микросервисов это также не
повлияет.

Под понятием "распределенность" я понимаю такое свойство, что если
микросервис A запущен на компьютере A1, а микросервис B запущен на компьютере B1
и компьютеры A1 и B1 связаны (“видят” друг друга), то найдётся такой
микросервис C, который будет отвечать за коммуникацию между ними (при
условии, что такая коммуникация предусмотрена). Причём микросервис С
может быть запущен на компьютерах A1, B1 и C1, если C1 связан с A1 и B1.

<b>en:</b>

Fault tolerance can be described in this way, in case of an error or failure
the operation of one service - this will not affect the operation of other
services in any way. Even if the service responsible for communication between
microservices and their synchronization,
the operation of other microservices is also not will affect.

By the concept of "distribution" I mean such a property that if
microservice A is running on computer A1 and microservice B is running on computer B1
and computers A1 and B1 are connected (“see” each other), then there is such
microservice C, which will be responsible for communication between them (with
provided such communication is provided). Moreover, microservice C
can be run on computers A1, B1 and C1 if C1 is connected to A1 and B1.

## 2. Доработка и поддержка/Improvement and support

<b>ru:</b>

Для <a href="https://github.com/K0001rch/Backupd">Backup-service</a> и Firewall-service планируется добавить общий
функционал для перечтения конфигурационного файла для того, чтобы можно
было применять изменения в режимах работы микросервисов в реальном
времени, это сделает сущности более “гибкими” и удобными в использовании.

Для Service-manager планируется реализовать режим синхронизации
подключаемых микросервисов для получения оперативной информации о
состояниях работающих сущностей.

В дальнейшей перспективе, планируется поддержать другие виды
резервных копирований и схем ротаций в сервисе Backup-service, а также
переписать парсинг и сериализацию конфигурационных атрибутов на C++ с
использованием библиотеки boost::serialization и поддержать конфигурирование
микросервисной системы информационной безопасности с помощью web-
интерфейса

<b>en:</b>
For <a href="https://github.com/K0001rch/Backupd">Backup-service</a> and Firewall-service, it is planned to add a common
functionality for rereading the configuration file so that you can
it was necessary to apply changes in the modes of operation of microservices in real
time, this will make the entities more “flexible” and easy to use.

It is planned to implement synchronization mode for Service-manager
pluggable microservices to get operational information about
states of running entities.

In the future, it is planned to support other types
backups and rotation schemes in the Backup-service, as well as
rewrite configuration attribute parsing and serialization in C++ with
using the boost::serialization library and support configuration
microservice information security system using web-
interface

## 3. Заключение/Conclusion

<b>ru:</b>

Данное программное обеспечение написано с использованием шести библиотек,
две из которых были разработаны в рамках данной задачи:
pthreads, libpcap, inotify, socket, queue, hash table.

Разработка протестирована не полностью и нуждается в дополнительных
стресс-тестах и функциональном тестировании, (желательно на различных
платформах), которое позволит выявить недоработки данной системы 
информационной безопасности и исправить их.

<b>en:</b>

This software is written using six libraries,
two of which were developed as part of this task:
pthreads, libpcap, inotify, socket, queue, hash table.

The development has not been fully tested and needs additional
stress tests and functional testing, (preferably on various
platforms), which will reveal the shortcomings of this system
information security and fix them.

## 4. Установка: сборка и запуск/Installation: making and launch

<b>ru:</b>

a) Произведите клонирование репозитория:
```
$ git clone git@github.com:K0001rch/Microservices-Security-System.git ~/MSS
```

b) Перейдите в директорию ~/MSS
```
$ cd ~/MSS
```

c) Выполните сборку и запустите цель check для того, чтобы убедиться в том, что
все микросервисы были собраны и готовы к запуску:
```
$ make && make check
```

Если всё прошло удачно, то в директории ~/MSS/bin будут лежать три микросервиса:
```
└  ~/MSS/bin  U › 𝗈𝗇 › main
 › file * | grep ELF
backup_service:   ELF 64-bit LSB pie executable, x86-64, version 1 (SYSV), dynamically linked, interpreter /lib64/ld-linux-x86-64.so.2, BuildID[sha1]=fdd44eca7030ba047af91ed1f184ba4ff8f70690, for GNU/Linux 4.4.0, with debug_info, not stripped
firewall_service: ELF 64-bit LSB pie executable, x86-64, version 1 (SYSV), dynamically linked, interpreter /lib64/ld-linux-x86-64.so.2, BuildID[sha1]=5a7d2a9f2a7a502d358b1adea756b80554872273, for GNU/Linux 4.4.0, with debug_info, not stripped
service_manager:  ELF 64-bit LSB pie executable, x86-64, version 1 (SYSV), dynamically linked, interpreter /lib64/ld-linux-x86-64.so.2, BuildID[sha1]=23a31fb7952caa80863081eda8f0df5299d53de0, for GNU/Linux 4.4.0, not stripped
```

d) По умолчанию, во всех прилагающихся микросервисах включено отображение дебажной информации.
Выключить её можно, отредактировав файл `~/MSS/include/helpers/helpers_common.h`
```
└  ~/MSS  U › 𝗈𝗇 › main
 › find ./ -name "helpers_common.h"
./include/helpers/helpers_common.h
```

Следующим образом:
```
diff --git a/include/helpers/helpers_common.h b/include/helpers/helpers_common.h
index d55e619..0a466fd 100644
--- a/include/helpers/helpers_common.h
+++ b/include/helpers/helpers_common.h
@@ -9,9 +9,9 @@
 #include <stdint.h>
 
 #define MSS_BUFFER_SIZE 512
-
+/*
 #define ENABLE_DEBUG_INFO
-
+*/
 #define MSS_PRINT_DEBUG(fmt, ...) \
        fprintf(stderr, fmt, ##__VA_ARGS__); fprintf(stderr, "\n"); 
```

в будущем, переключение в режим debug и из него, будет сделано на этапе сборки.

e) Запустите сначала service_manager, а также другие микросервисы, предварительно сконфигурировав
их.

```
$ cd ~/MSS/bin
$ ./service_manager 
```

Запуск backup-service осуществляется с параметром argv,
которое имеет значение ip-адреса, на котором работает
`Service-manager`
(конфигурацию смотри здесь `~/MSS/bin/services/backup_service/config.conf`):
```
$ ./backup_service 192.168.0.10
```

Аналогичным образом запускается firewall-service.
Однако, ему нужно предоставить права суперпользователя
для того, чтобы он мог начать перехватывать трафик
с указанного интерфейса в конфигурационном файле 
(Конфигурацию смотри здесь: `/home/k1rch/MSS/bin/services/firewall_service/config.conf`).
```
# ./firewall_service 192.168.0.10
```

<b>en:</b>
a) Clone the repository:
```
$ git clone git@github.com:K0001rch/Microservices-Security-System.git ~/MSS
```

b) Go to ~/MSS directory:
```
$ cd ~/MSS
```

c) Build and run the check target to ensure that
all microservices have been assembled and are ready to run:
```
$ make && make check
```

If everything went well, then the ~/MSS/bin directory will contain three microservices:
```
└  ~/MSS/bin  U › 𝗈𝗇 › main
 › file * | grep ELF
backup_service:   ELF 64-bit LSB pie executable, x86-64, version 1 (SYSV), dynamically linked, interpreter /lib64/
ld-linux-x86-64.so.2, BuildID[sha1]=fdd44eca7030ba047af91ed1f184ba4ff8f70690, for GNU/Linux 4.4.0, with debug_info, 
not stripped
firewall_service: ELF 64-bit LSB pie executable, x86-64, version 1 (SYSV), dynamically linked, interpreter /lib64/
ld-linux-x86-64.so.2, BuildID[sha1]=5a7d2a9f2a7a502d358b1adea756b80554872273, for GNU/Linux 4.4.0, with debug_info, 
not stripped
service_manager:  ELF 64-bit LSB pie executable, x86-64, version 1 (SYSV), dynamically linked, interpreter /lib64/
ld-linux-x86-64.so.2, BuildID[sha1]=23a31fb7952caa80863081eda8f0df5299d53de0, for GNU/Linux 4.4.0, not stripped
```

d) By default, display of debug information is enabled in all attached microservices.
You can turn it off by editing the file `~/MSS/include/helpers/helpers_common.h`
```
└  ~/MSS  U › 𝗈𝗇 › main
 › find ./ -name "helpers_common.h"
./include/helpers/helpers_common.h
```

Следующим образом:
```
diff --git a/include/helpers/helpers_common.h b/include/helpers/helpers_common.h
index d55e619..0a466fd 100644
--- a/include/helpers/helpers_common.h
+++ b/include/helpers/helpers_common.h
@@ -9,9 +9,9 @@
 #include <stdint.h>
 
 #define MSS_BUFFER_SIZE 512
-
+/*
 #define ENABLE_DEBUG_INFO
-
+*/
 #define MSS_PRINT_DEBUG(fmt, ...) \
        fprintf(stderr, fmt, ##__VA_ARGS__); fprintf(stderr, "\n"); 
```
in the future, switching to and from debug mode will be done during the build phase.

e) Start service_manager first, as well as other microservices, after configuring
their.

```
$ cd ~/MSS/bin
$ ./service_manager
```

The backup-service is started with the argv parameter,
which has the value of the ip-address on which it works
`Service-manager`
(see `~/MSS/bin/services/backup_service/config.conf` for configuration):
```
$ ./backup_service 192.168.0.10
```

The firewall-service is started in the same way.
However, he needs to be granted superuser rights.
so that it can start intercepting traffic
from the specified interface in the configuration file
(See the configuration here: `/home/k1rch/MSS/bin/services/firewall_service/config.conf`).
```
# ./firewall_service 192.168.0.10
```
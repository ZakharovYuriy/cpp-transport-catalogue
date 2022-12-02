Установка Protobuf:


Скачать Protobuf можно с репозитория на [GitHub](https://github.com/protocolbuffers/protobuf/releases).<br>
 + Выберите архив protobuf-cpp с исходным кодом последней версии и распакуйте его на своём компьютере. Исходный код содержит CMake-проект.<br>
 + Создадим папки build-debug и build-release для сборки двух конфигураций Protobuf. Если вы используете Visual Studio, будет достаточно одной папки build.<br>
 
 ```
Protobuf доступен в пакетах некоторых операционных систем.
Например, в Debian-системах его можно установить 
через менеджер пакетов apt, а не собирать самостоятельно. 
В этом случае использование библиотеки будет отличаться, 
поэтому лучше всё же скачать исходный код и собрать вручную.
```
 + Заранее создадим папку, в которой разместим пакет Protobuf. Будем называть её /path/to/protobuf/package.<br>
Если вы собираете не через IDE, в папке build-debug выполните следующие команды:<br>

```
cmake <путь к protobuf>/cmake -DCMAKE_BUILD_TYPE=Debug \
      -Dprotobuf_BUILD_TESTS=OFF \
      -DCMAKE_INSTALL_PREFIX=/path/to/protobuf/package
cmake --build . 
```
 + Дополнительные параметры:<br>
-Dprotobuf_BUILD_TESTS=OFF — чтобы не тратить время на сборку тестов,<br>
:exclamation:<br>
-DCMAKE_INSTALL_PREFIX=/path/to/protobuf/package — чтобы сообщить, где нужно будет создать пакет Protobuf.<br>
Для Visual Studio команды немного другие. Конфигурация указывается не при генерации, а при сборке:<br>

```
cmake <путь к protobuf>/cmake ^
      -Dprotobuf_BUILD_TESTS=OFF ^
      -DCMAKE_INSTALL_PREFIX=/path/to/protobuf/package ^
      -Dprotobuf_MSVC_STATIC_RUNTIME=OFF
cmake --build . --config Debug 
```
Задать переменную protobuf_MSVC_STATIC_RUNTIME при сборке под Visual Studio очень важно. Дело в том, что Visual Studio имеет два набора Runtime-библиотек — библиотек, которые компонуются с каждой собираемой программой. Если окажется, что два бинарных файла зависят от библиотек разных наборов, получите головную боль из-за криптографических ошибок во время компоновки приложения.<br>

 + Итак, библиотека готова. Теперь запустим волшебную команду:<br>

```
cmake --install . 
```
Под Visual Studio нужно указать конфигурацию, поскольку она не задавалась во время генерации:<br>

```
cmake --install . --config Debug 
```
И CMake скопирует все необходимые файлы в заранее подготовленное место /path/to/protobuf/package.<br>

 + Проделываем те же шаги с конфигурацией Release. Устанавливать будем в ту же папку. Так как статические библиотеки, собранные в конфигурации Debug, имеют суффикс d, конфликта не возникнет.<br>

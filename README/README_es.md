# CardputerADV-DensyaNavi — Herramientas para PC

[日本語](README.md) | [English](README_en.md) | [한국어](README_ko.md) | [繁體中文](README_zh-TW.md) | [简体中文](README_zh-CN.md) | [Español](README_es.md) | [Français](README_fr.md)

Esta carpeta contiene herramientas para PC destinadas a tareas como descargar datos de estaciones ferroviarias.

## Descargar datos de estaciones

### Requisitos
- Python (se recomienda Python 3.10 o posterior)
- Conexión a Internet

No es necesario instalar paquetes adicionales. El script utiliza únicamente la biblioteca estándar de Python.

### Cómo ejecutarlo

1. Descarga el archivo ZIP de la [rama `pc`](https://github.com/marimutukisinnmu-rgb/CardputerADV-DensyaNavi/tree/pc) desde GitHub.
2. Descomprime el archivo ZIP.
3. Abre una terminal en la carpeta que contiene `download_data.py` y ejecuta el comando correspondiente.

**Windows**

Si está disponible el comando `py` de Python Launcher:

```cmd
py download_data.py
```

También puedes ejecutar lo siguiente si Python está incluido en PATH:

```cmd
python download_data.py
```

Puedes usar tanto el Símbolo del sistema como PowerShell.

**macOS / Linux**

Si Python 3 está incluido en PATH, ejecuta:

```sh
python3 download_data.py
```

### Estructura después de la descarga

Si la ejecución termina correctamente, se creará una carpeta `station_data/` junto al script, con este contenido:

```text
station_data/
├── out/
├── README.md
└── LICENSE.md
```

Los datos se descargan de la rama `main` de [Seo-4d696b75/station_database](https://github.com/Seo-4d696b75/station_database). No es necesario instalar Git.

### Funciones actuales del script
- Descarga un archivo ZIP desde GitHub
- Extrae el directorio `out/`
- Guarda los archivos `README.md` y `LICENSE.md` del proyecto original

Por ahora, el script no procesa ni convierte los datos y tampoco escribe en una tarjeta microSD.

### Licencia y notas

Consulta `station_data/LICENSE.md` y `station_data/README.md` para conocer la licencia, la atribución y las condiciones de uso de los datos originales. El repositorio de origen advierte que no garantiza la exactitud ni la actualidad de los datos.

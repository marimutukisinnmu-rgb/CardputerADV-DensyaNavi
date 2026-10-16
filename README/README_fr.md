# CardputerADV-DensyaNavi — Outils PC

[日本語](README.md) | [English](README_en.md) | [한국어](README_ko.md) | [繁體中文](README_zh-TW.md) | [简体中文](README_zh-CN.md) | [Español](README_es.md) | [Français](README_fr.md)

Ce dossier contient des outils pour PC, notamment pour télécharger les données des gares ferroviaires.

## Télécharger les données des gares

### Prérequis
- Python (version 3.10 ou ultérieure recommandée)
- Une connexion Internet

Aucun paquet supplémentaire n'est nécessaire. Le script utilise uniquement la bibliothèque standard de Python.

### Exécution

1. Téléchargez l'archive ZIP de la [branche `pc`](https://github.com/marimutukisinnmu-rgb/CardputerADV-DensyaNavi/tree/pc) depuis GitHub.
2. Décompressez l'archive ZIP.
3. Ouvrez un terminal dans le dossier contenant `download_data.py` et exécutez la commande appropriée ci-dessous.

**Windows**

Si la commande `py` de Python Launcher est disponible :

```cmd
py download_data.py
```

Vous pouvez également utiliser cette commande si Python est accessible via PATH :

```cmd
python download_data.py
```

L'invite de commandes et PowerShell sont tous deux compatibles.

**macOS / Linux**

Si Python 3 est accessible via PATH, exécutez :

```sh
python3 download_data.py
```

### Structure après téléchargement

En cas de réussite, un dossier `station_data/` est créé à côté du script et contient :

```text
station_data/
├── out/
├── README.md
└── LICENSE.md
```

Les données sont téléchargées depuis la branche `main` de [Seo-4d696b75/station_database](https://github.com/Seo-4d696b75/station_database). Git n'a pas besoin d'être installé.

### Fonctions actuelles du script
- Téléchargement d'une archive ZIP depuis GitHub
- Extraction du répertoire `out/`
- Enregistrement des fichiers `README.md` et `LICENSE.md` du projet source

Le script ne traite ni ne convertit les données pour le moment et n'écrit rien sur une carte microSD.

### Licence et remarques

Consultez `station_data/LICENSE.md` et `station_data/README.md` pour connaître la licence, l'attribution et les conditions d'utilisation des données sources. Le dépôt d'origine précise qu'il ne garantit ni l'exactitude ni l'actualité des données.

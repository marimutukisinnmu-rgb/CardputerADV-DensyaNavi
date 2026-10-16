# CardputerADV-DensyaNavi — PC 도구

[日本語](README.md) | [English](README_en.md) | [한국어](README_ko.md) | [繁體中文](README_zh-TW.md) | [简体中文](README_zh-CN.md) | [Español](README_es.md) | [Français](README_fr.md)

이 폴더에는 철도역 데이터 다운로드 등의 작업을 위한 PC용 도구가 들어 있습니다.

## 역 데이터 다운로드

### 준비물
- Python (Python 3.10 이상 권장)
- 인터넷 연결

추가 패키지를 설치할 필요가 없습니다. 이 스크립트는 Python 표준 라이브러리만 사용합니다.

### 실행 방법

1. GitHub에서 [`pc` 브랜치](https://github.com/marimutukisinnmu-rgb/CardputerADV-DensyaNavi/tree/pc)의 ZIP 파일을 다운로드합니다.
2. ZIP 파일의 압축을 풉니다.
3. `download_data.py`가 있는 폴더에서 터미널을 열고 아래 명령 중 해당하는 것을 실행합니다.

**Windows**

Python Launcher의 `py` 명령을 사용할 수 있다면:

```cmd
py download_data.py
```

또는 Python이 PATH에 등록되어 있다면:

```cmd
python download_data.py
```

명령 프롬프트와 PowerShell 모두 사용할 수 있습니다.

**macOS / Linux**

Python 3이 PATH에 등록되어 있다면 다음을 실행합니다.

```sh
python3 download_data.py
```

### 다운로드 후 폴더 구조

성공하면 스크립트와 같은 위치에 `station_data/` 폴더가 생성되며, 다음 내용이 저장됩니다.

```text
station_data/
├── out/
├── README.md
└── LICENSE.md
```

데이터는 [Seo-4d696b75/station_database](https://github.com/Seo-4d696b75/station_database)의 `main` 브랜치에서 다운로드합니다. Git을 설치할 필요는 없습니다.

### 현재 스크립트의 기능
- GitHub ZIP 파일 다운로드
- `out/` 디렉터리 압축 해제
- 원본 프로젝트의 `README.md`와 `LICENSE.md` 저장

현재 데이터 가공이나 변환, microSD 카드 기록은 수행하지 않습니다.

### 라이선스 및 참고 사항

원본 데이터의 라이선스, 출처 및 이용 조건은 `station_data/LICENSE.md`와 `station_data/README.md`를 확인하세요. 원본 저장소는 데이터의 정확성이나 최신성을 보장하지 않는다고 안내합니다.

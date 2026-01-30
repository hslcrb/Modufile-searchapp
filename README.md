# 모두파일 (Modufile)

**모두파일(Modufile)**은 기존의 그 어떤 파일 검색 도구보다 더 강력하고 아름다운 초고속 파일 검색 유틸리티입니다. Windows의 전설적인 프로그램 'Everything'을 뛰어넘는 성능과 사용자 경험을 목표로 하며, Rust로 작성되어 극강의 가벼움과 속도를 자랑합니다.

## ✨ 왜 Everything보다 좋은가?

1. **알잘딱 (Smart Match)**: 단순히 텍스트가 포함된 파일을 찾는 것을 넘어, 사용자의 의도를 파악하는 '퍼지 매칭(Fuzzy Matching)' 알고리즘을 탑재했습니다. 오타가 나거나 검색어가 불완전해도 **알아서 잘 딱 깔끔하고 센스있게** 원하는 파일을 찾아줍니다.
2. **진정한 크로스 플랫폼**: Windows에 국한되지 않고 Linux, macOS 등 모든 OS에서 완벽하게 동일한 초고속 성능을 제공합니다.
3. **독보적인 에어로(Aero) 디자인**: 칙칙한 구식 UI가 아닙니다. 현대적인 글래스모피즘(Glassmorphism)과 미니멀리즘이 결합된 에어로 UI는 사용하는 것만으로도 즐거움을 줍니다.
4. **미친 속도의 병렬 인덱싱**: `jwalk`와 `rayon`을 활용한 멀티코어 병렬 탐색 기술로, 수십만 개의 파일을 단 몇 초 만에 인덱싱합니다.

## 🌟 주요 특징
- **초광속 검색**: 인덱싱된 데이터를 기반으로 입력과 동시에 실시간 결과 출력.
- **다국어 지원**: 한국어, 영어, 일본어, 중국어(번체)를 완벽 지원하며 시스템 언어에 맞춰 자동 설정됩니다.
- **다크/라이트 모드**: 사용자의 눈 건강과 취향을 고려한 세련된 테마 전환.
- **오픈소스**: 투명한 코드와 커뮤니티 중심의 발전.

## 🚀 시작하기

### 설치 및 빌드
Rust와 Node.js가 설치되어 있어야 합니다.

```bash
# 의존성 설치
npm install

# 개발 모드 실행
npm run tauri dev

# 프로덕션 빌드
npm run tauri build
```

## 🛠 기술 스택
- **Backend**: [Rust](https://www.rust-lang.org/) (High Performance Parallel Processing)
- **Frontend**: [Tauri](https://tauri.app/), JavaScript, CSS (Minimalist Aero UI)
- **Matching**: `fuzzy-matcher` (Smart Match Algorithm)

## 📄 라이선스
MIT License

## ✍️ 제작자
**Rheehose (Rhee Creative)**  
© 2008-2026 Rhee Creative. All rights reserved.

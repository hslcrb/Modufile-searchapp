# 모두파일 (Modufile)

**모두파일(Modufile)**은 컴퓨터 내의 모든 파일을 눈 깜짝할 새에 찾아주는 초고속 파일 검색 유틸리티입니다. Windows의 'Everything'과 유사한 사용자 경험을 제공하며, Rust로 작성되어 가볍고 빠릅니다.

## ✨ 주요 특징
- **초광속 검색**: 수십만 개의 파일을 단 몇 밀리초 만에 필터링합니다.
- **간결한 GUI**: 복잡한 설정 없이 검색창 하나로 모든 것을 해결합니다.
- **OS 독립적**: Rust와 Tauri를 사용하여 Linux, Windows, macOS 어디서든 동일하게 작동합니다.
- **미려한 디자인**: 현대적인 다크 모드와 글래스모피즘 스타일의 UI를 제공합니다.

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
- **Backend**: [Rust](https://www.rust-lang.org/)
- **Frontend**: [Tauri](https://tauri.app/), JavaScript, CSS (Vanilla)
- **Indexing**: `walkdir` 기반 고속 탐색 및 메모리 기반 인덱싱

## 📄 라이선스
MIT License

## ✍️ 제작자
**Rheehose (Rhee Creative)**  
© 2008-2026 Rhee Creative. All rights reserved.

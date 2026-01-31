# 모두파일 (Modufile) - Native C++ Edition

**모두파일(Modufile)**은 기존의 그 어떤 파일 검색 도구보다 더 강력하고 아름다운 초고속 파일 검색 유틸리티입니다. 

기존의 Rust/Tauri 구조에서 발생하는 런타임 오버헤드를 완전히 제거하고, **순수 C++ 17**과 **Qt 6 Framework**를 사용하여 **100% 네이티브 애플리케이션**으로 재탄생했습니다. Windows의 'Everything'을 능가하는 성능과 현대적인 사용자 경험을 제공합니다.

## ✨ 왜 기존 버전보다 좋은가?

1.  **순수 C++ 네이티브**: 웹 엔진(WebView)을 사용하지 않습니다. 덕분에 메모리 점유율은 극도로 낮아졌고, 응답 속도는 물리적 한계까지 끌어올렸습니다.
2.  **독자적인 알잘딱 (Smart Match) 엔진**: C++로 최적화된 퍼지 매칭 알고리즘을 탑재했습니다. 검색어가 불완전하거나 오타가 있어도 **알아서 잘 딱 깔끔하고 센스있게** 원하는 파일을 찾아줍니다.
3.  **OS 독립적 성능**: Windows, Linux, macOS에서 각 운영체제의 네이티브 API를 직접 호출하여 최적의 성능을 보장합니다.
4.  **표준화된 자동화 빌드**: GitHub Actions를 통해 3대 OS 전용 실행 파일과 Docker 컨테이너 이미지를 실시간으로 배포합니다.

## 🌟 주요 특징

-   **초광속 검색**: 인덱싱된 데이터를 기반으로 입력과 동시에 실시간 결과 출력.
-   **에어로(Aero) 디자인 철학**: 네이티브 위젯을 활용하면서도 세련된 스타일링을 적용하여 미니멀리즘과 성능을 동시에 잡았습니다.
-   **병렬 인덱싱**: `std::filesystem`과 멀티스레딩 기술을 활용하여 수백만 개의 파일을 단 몇 초 만에 인덱싱합니다.
-   **다국어 및 테마 지원**: 네이티브 수준에서 구현된 다국어 처리와 다크 모드.

## 🚀 시작하기

### 실행 파일 다운로드
[Releases](https://github.com/hslcrb/Modufile-searchapp/releases) 탭에서 각 OS에 맞는 최신 빌드(`latest`)를 다운로드하여 즉시 사용할 수 있습니다.

### Docker 이용
```bash
docker pull ghcr.io/hslcrb/modufile:latest
```

### 소스 빌드 (개발자용)
-   Qt 6.5.3 이상 환경이 필요합니다.
-   CMake 3.16 이상이 필요합니다.

```bash
# 빌드 구성
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release

# 빌드 실행
cmake --build build --config Release
```

## 🛠 기술 스택

-   **Language**: Pure C++ 17
-   **GUI Framework**: [Qt 6.5+](https://www.qt.io/)
-   **Build System**: CMake
-   **CI/CD**: GitHub Actions (Windows, macOS, Linux, Docker GHCR)

## 📄 라이선스
MIT License

## ✍️ 제작자
**Rheehose (Rhee Creative)**  
© 2008-2026 Rhee Creative. All rights reserved.

# STM32F767 빌드 안내

현재 CLI 빌드 방법과 메모리 구성은 [README](../README.md)를 참고하세요.

```bash
make stm32-build MODE=register EXAMPLE=led
make stm32-build MODE=hal EXAMPLE=adc
make check-all
make build-all
make clean
```

`Core/` 디렉터리와 CubeMX 생성 파일은 필요하지 않습니다.
이전 `EXAMPLE=core`, `USE_HAL`, `stm32-run` 인터페이스는 사용하지 않습니다.
산출물은 `target/<MODE>/<EXAMPLE>/`에 생성됩니다.

# STM32

### STM32 NUCLEO-F767ZI — test6

보드의 **ST-LINK USB 커넥터**를 연결하고 저장소 루트에서 실행한다.

```bash
make stm32-build            # 보드 없이 빌드
make stm32-run              # ST-LINK로 업로드하고 실행
```

USER 버튼(B1)을 누를 때마다 초록 LED(LD1)가 켜지거나 꺼진다. 별도 배선은 필요 없다.
`probe-rs` 설치 등 자세한 준비는 [STM32 실행 안내](stm32_rs/README.md)를 참고한다.
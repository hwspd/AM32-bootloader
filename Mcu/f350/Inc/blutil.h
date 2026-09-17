/* MCU-specific utility functions for the GD32F350 bootloader. */
#pragma once

#define RAM_BASE 0x20000000U
#define RAM_SIZE (8U * 1024U)

#define BOARD_FLASH_SIZE 64

#define GPIO_PIN(n) (1U << (n))

#define GPIO_PULL_NONE GPIO_PUPD_NONE
#define GPIO_PULL_UP GPIO_PUPD_PULLUP
#define GPIO_PULL_DOWN GPIO_PUPD_PULLDOWN

#define GPIO_OUTPUT_PUSH_PULL GPIO_OTYPE_PP

static inline void gpio_mode_set_input(uint32_t pin, uint32_t pull_up_down)
{
  gpio_mode_set(input_port, GPIO_MODE_INPUT, pull_up_down, pin);
}

static inline void gpio_mode_set_output(uint32_t pin, uint32_t output_mode)
{
  gpio_mode_set(input_port, GPIO_MODE_OUTPUT, output_mode, pin);
}

static inline void gpio_set(uint32_t pin)
{
  gpio_bit_set(input_port, pin);
}

static inline void gpio_clear(uint32_t pin)
{
  gpio_bit_reset(input_port, pin);
}

static inline bool gpio_read(uint32_t pin)
{
  return (gpio_input_port_get(input_port) & pin) != 0;
}

#define BL_TIMER TIMER16

/* APB2 runs at 36 MHz; its timer clock is doubled to 72 MHz. */
static inline void bl_timer_init(void)
{
  rcu_periph_clock_enable(RCU_TIMER16);
  TIMER_CAR(BL_TIMER) = 0xFFFFU;
  TIMER_PSC(BL_TIMER) = 71U;
  timer_auto_reload_shadow_enable(BL_TIMER);
  timer_enable(BL_TIMER);
}

static inline void bl_timer_disable(void)
{
  timer_disable(BL_TIMER);
  rcu_periph_clock_disable(RCU_TIMER16);
}

static inline uint16_t bl_timer_us(void)
{
  return timer_counter_read(BL_TIMER);
}

static inline void bl_clock_config(void)
{
  rcu_periph_clock_enable(RCU_GPIOA);
  rcu_periph_clock_enable(RCU_GPIOB);
}

static inline void bl_gpio_init(void)
{
}

static inline bool bl_was_software_reset(void)
{
  return (RCU_RSTSCK & RCU_RSTSCK_SWRSTF) != 0U;
}

static inline void jump_to_application(void)
{
  __disable_irq();
  SysTick->CTRL = 0U;
  SysTick->LOAD = 0U;
  SysTick->VAL = 0U;
  bl_timer_disable();

  const uint32_t app_address = MCU_FLASH_START + FIRMWARE_RELATIVE_START;
  const uint32_t* app_data = (const uint32_t*)app_address;
  const uint32_t stack_top = app_data[0];
  const uint32_t jump_address = app_data[1];

  SCB->VTOR = app_address;
  __DSB();
  __ISB();

  asm volatile(
    "msr control, %2\n"
    "mov sp, %0\n"
    "msr msp, %0\n"
    "isb\n"
    "bx %1\n"
    :
    : "r"(stack_top), "r"(jump_address), "r"(0U)
    : "memory");
}

void SysTick_Handler(void)
{
}

#define RCU_MODIFY(__delay)                 \
  do {                                      \
    volatile uint32_t i;                    \
    if (0U != (__delay)) {                  \
      RCU_CFG0 |= RCU_AHB_CKSYS_DIV2;       \
      for (i = 0U; i < (__delay); i++) {    \
      }                                     \
      RCU_CFG0 |= RCU_AHB_CKSYS_DIV4;       \
      for (i = 0U; i < (__delay); i++) {    \
      }                                     \
    }                                       \
  } while (0)

static void system_clock_72m_irc8m(void)
{
  FMC_WS = (FMC_WS & ~FMC_WS_WSCNT) | WS_WSCNT_2;

  RCU_CFG0 |= RCU_AHB_CKSYS_DIV1;
  RCU_CFG0 |= RCU_APB2_CKAHB_DIV2;
  RCU_CFG0 |= RCU_APB1_CKAHB_DIV2;

  RCU_CFG0 &= ~(RCU_CFG0_PLLSEL | RCU_CFG0_PLLMF |
                RCU_CFG0_PLLMF4 | RCU_CFG0_PLLPREDV);
  RCU_CFG1 &= ~(RCU_CFG1_PLLPRESEL | RCU_CFG1_PLLMF5 | RCU_CFG1_PREDV);
  RCU_CFG0 |= RCU_PLLSRC_IRC8M_DIV2 |
              (RCU_PLL_MUL18 & ~RCU_CFG1_PLLMF5);
  RCU_CFG1 |= RCU_PLL_MUL18 & RCU_CFG1_PLLMF5;

  RCU_CTL0 |= RCU_CTL0_PLLEN;
  while (0U == (RCU_CTL0 & RCU_CTL0_PLLSTB)) {
  }

  RCU_CFG0 = (RCU_CFG0 & ~RCU_CFG0_SCS) | RCU_CKSYSSRC_PLL;
  while (0U == (RCU_CFG0 & RCU_SCSS_PLL)) {
  }
}

void SystemInit(void)
{
  RCU_CTL0 |= RCU_CTL0_IRC8MEN;
  while (0U == (RCU_CTL0 & RCU_CTL0_IRC8MSTB)) {
  }

  RCU_MODIFY(0x80U);
  RCU_CFG0 &= ~RCU_CFG0_SCS;
  RCU_CTL0 &= ~(RCU_CTL0_HXTALEN | RCU_CTL0_CKMEN |
                RCU_CTL0_PLLEN | RCU_CTL0_HXTALBPS);

  RCU_CFG0 &= ~(RCU_CFG0_SCS | RCU_CFG0_AHBPSC | RCU_CFG0_APB1PSC |
                RCU_CFG0_APB2PSC | RCU_CFG0_ADCPSC | RCU_CFG0_CKOUTSEL |
                RCU_CFG0_CKOUTDIV | RCU_CFG0_PLLDV | RCU_CFG0_PLLSEL |
                RCU_CFG0_PLLMF | RCU_CFG0_PLLMF4 | RCU_CFG0_PLLPREDV |
                RCU_CFG0_USBFSPSC);
  RCU_CFG1 &= ~(RCU_CFG1_PREDV | RCU_CFG1_PLLMF5 | RCU_CFG1_PLLPRESEL);
  RCU_CFG2 &= ~(RCU_CFG2_CECSEL | RCU_CFG2_USBFSPSC2 |
                RCU_CFG2_USART0SEL | RCU_CFG2_ADCSEL |
                RCU_CFG2_IRC28MDIV | RCU_CFG2_ADCPSC2);
  RCU_CTL1 &= ~RCU_CTL1_IRC28MEN;
  RCU_ADDCTL &= ~RCU_ADDCTL_IRC48MEN;
  RCU_INT = 0x00000000U;
  RCU_ADDINT = 0x00000000U;

  system_clock_72m_irc8m();
}

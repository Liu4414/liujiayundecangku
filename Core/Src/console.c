#include "config.h"
#include "console.h"
#include "dbg.h"
#include "motor.h"
#include "servo.h"
#include "app.h"

#define RX_SIZE 64

static volatile uint8_t  s_rx[RX_SIZE];
static volatile uint16_t s_head;
static volatile uint16_t s_tail;

static char    s_line[32];
static uint8_t s_len;

/* ---------------- 环形缓冲 ---------------- */
void console_init(void)
{
    s_head = 0;
    s_tail = 0;
    s_len  = 0;
}

void console_rx_byte(uint8_t b)
{
    uint16_t next = (uint16_t)((s_head + 1u) % RX_SIZE);

    if (next != s_tail) {          /* 满了就丢 */
        s_rx[s_head] = b;
        s_head = next;
    }
}

/* ---------------- 解析工具 ---------------- */
static uint8_t parse_int(const char **ps, int *out)
{
    const char *p = *ps;
    int sign = 1;
    int val = 0;
    uint8_t any = 0;

    while ((*p == ' ') || (*p == '\t') || (*p == ',') || (*p == ':') || (*p == '=')) {
        p++;
    }
    if (*p == '-') { sign = -1; p++; }
    else if (*p == '+') { p++; }

    while ((*p >= '0') && (*p <= '9')) {
        val = val * 10 + (*p - '0');
        if (val > 100000) val = 100000;
        any = 1;
        p++;
    }

    *ps = p;
    *out = sign * val;
    return any;
}

static uint8_t parse2(const char **ps, int *a, int *b)
{
    if (!parse_int(ps, a)) return 0;
    if (!parse_int(ps, b)) return 0;
    return 1;
}

/* ---------------- 帮助 ---------------- */
static void print_help(void)
{
    dbg_printf("\r\n===== SMART CAR 命令 =====\r\n");
    dbg_printf(" ?            帮助\r\n");
    dbg_printf(" m <1-6> <v>  单电机  v=-1000~1000  例: m 1 300\r\n");
    dbg_printf(" a <v>        六电机同速            例: a 200\r\n");
    dbg_printf(" s            六电机滑行停止\r\n");
    dbg_printf(" b            六电机短路制动\r\n");
    dbg_printf(" e / d        使能 / 切断 STBY\r\n");
    dbg_printf(" sv <1-5> <p> 舵机 p=-100~100       例: sv 1 50\r\n");
    dbg_printf(" st           打印状态\r\n");
    dbg_printf(" x            退出手动，回到遥控模式\r\n");
    dbg_printf("===========================\r\n");
}

static const char *state_name(app_state_t s)
{
    switch (s) {
    case APP_RUN:      return "RUN";
    case APP_ESTOP:    return "ESTOP";
    case APP_BRAKE:    return "BRAKE";
    default:           return "FAILSAFE";
    }
}

/* ---------------- 执行一条命令 ---------------- */
static void exec(const char *s)
{
    char c = *s++;

    while (*s == ' ') s++;

    switch (c) {
    case '?':
        print_help();
        break;

    case 'm': case 'M': {
        int ch, v;
        if (!parse2(&s, &ch, &v)) {
            dbg_printf("ERR 格式: m <1-6> <v>\r\n");
            break;
        }
        if ((ch < 1) || (ch > 6)) {
            dbg_printf("ERR 通道 1~6\r\n");
            break;
        }
        app_set_manual(1);
        motor_set((motor_id_t)(ch - 1), (int16_t)v);
        dbg_printf("M%d = %d\r\n", ch, motor_get((motor_id_t)(ch - 1)));
        break;
    }

    case 'a': case 'A': {
        int v, i;
        if (!parse_int(&s, &v)) {
            dbg_printf("ERR 格式: a <v>\r\n");
            break;
        }
        app_set_manual(1);
        for (i = 0; i < MOTOR_NUM; i++) {
            motor_set((motor_id_t)i, (int16_t)v);
        }
        dbg_printf("ALL = %d\r\n", v);
        break;
    }

    case 'b': case 'B': {
        int i;
        app_set_manual(1);
        for (i = 0; i < MOTOR_NUM; i++) {
            motor_brake((motor_id_t)i);
        }
        dbg_printf("BRAKE\r\n");
        break;
    }

    case 'e': case 'E':
        motor_enable(1);
        dbg_printf("STBY = ON\r\n");
        break;

    case 'd': case 'D':
        motor_enable(0);
        dbg_printf("STBY = OFF\r\n");
        break;

    case 's': case 'S':
        if ((*s == 'v') || (*s == 'V')) {
            int ch, p;
            s++;
            if (!parse2(&s, &ch, &p)) {
                dbg_printf("ERR 格式: sv <1-5> <p>\r\n");
                break;
            }
            if ((ch < 1) || (ch > SERVO_NUM)) {
                dbg_printf("ERR 通道 1~5\r\n");
                break;
            }
            app_set_manual(1);
            servo_set_norm((uint8_t)(ch - 1), (int16_t)p);
            dbg_printf("SERVO%d = %d\r\n", ch, p);
        } else if ((*s == 't') || (*s == 'T')) {
            int i;
            dbg_printf("state=%s manual=%d\r\n", state_name(app_state()), 1);
            for (i = 0; i < MOTOR_NUM; i++) {
                dbg_printf(" M%d=%d", i + 1, motor_get((motor_id_t)i));
            }
            dbg_printf("\r\n servo_ok=%d\r\n", servo_ok());
        } else {
            app_set_manual(1);
            motor_stop_all();
            dbg_printf("STOP\r\n");
        }
        break;

    case 'x': case 'X':
        app_set_manual(0);
        dbg_printf("REMOTE\r\n");
        break;

    default:
        dbg_printf("ERR 未知命令 '%c'，发 ? 看帮助\r\n", c);
        break;
    }
}

/* ---------------- 主循环任务 ---------------- */
void console_task(void)
{
    while (s_tail != s_head) {
        uint8_t b = s_rx[s_tail];

        s_tail = (uint16_t)((s_tail + 1u) % RX_SIZE);

        if ((b == '\r') || (b == '\n')) {
            if (s_len > 0) {
                s_line[s_len] = '\0';
                exec(s_line);
                s_len = 0;
            }
        } else if ((b == 0x08u) || (b == 0x7Fu)) {
            if (s_len > 0) s_len--;
        } else if (s_len < (uint8_t)(sizeof(s_line) - 1u)) {
            s_line[s_len++] = (char)b;
        } else {
            s_len = 0;
        }
    }
}

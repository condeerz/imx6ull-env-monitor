# i.MX6ULL 机柜动环监控节点（LVGL 8.4）

基于 NXP i.MX6ULL（Cortex-A7）与 Linux 4.1.15 的机柜环境监控终端：四个线程完成
「传感器采集 → 应用层邮箱分发 → 显示 / 存储 / 上传」，界面用 LVGL 8.4 直接渲染到
`/dev/fb0`，数据落 SQLite 并通过 MQTT 上云。

## 架构

```text
  /dev/dht11_misc   ┐
  /dev/bh1750_misc  ├─► collect_thread ──mailbox──┬─► lvgl_thread    (LVGL 8.4 → /dev/fb0)
  烟雾通道          ┘   （1 s 汇总一轮）          ├─► sqlite_thread  (本地落盘)
                                                 └─► mqtt_thread    (paho.mqtt.c → OneNET)
```

- 采集节奏：DHT11 每 2 s、BH1750 每 3 s、烟雾每 5 s；汇总线程每 1 s 分发一轮
- 通信层三层：`mailbox.c`（按线程名寻址）→ `linklist.c`（任务链表）→ `linkqueue.c`（每线程一条链式队列）
- 队列内置互斥锁 + 条件变量，提供 `try_linkqueue()` / `wait_linkqueue()` / `close_linkqueue()`

## 目录

| 路径 | 说明 |
|---|---|
| `main.c` | 初始化日志与邮箱，注册并启动四个线程 |
| `collect.c` | 三路传感器采集 + 1 s 汇总分发 |
| `lvgl_user.c` / `ui*.c` / `screens/` | LVGL 界面、事件与样式 |
| `sqlite.c` | 收到消息后写入本地数据库 |
| `mqtt.c` | 收到消息后经 paho.mqtt.c 发布 |
| `mailbox.c` / `linklist.c` / `linkqueue.c` | 自研线程间通信三层 |
| `lv_conf.h` / `lv_drv_conf.h` | LVGL 与显示 / 输入驱动配置 |
| `lv_drivers/` | LVGL Linux 驱动（fbdev / evdev） |
| `Makefile` | arm-linux-gnueabihf 交叉编译脚本 |

## 构建

依赖：Linaro `arm-linux-gnueabihf` 4.9.4 工具链、LVGL 8.4 源码、sqlite3 / paho.mqtt.c / OpenSSL
交叉编译库（第三方依赖不随仓库提供）。

```bash
# LVGL 8.4 源码放到工程根的 lvgl/ 下
git clone -b v8.4 https://github.com/lvgl/lvgl.git lvgl

# 交叉编译
export PATH=$HOME/tools/opt/gcc-linaro-4.9.4-2017.01-x86_64_arm-linux-gnueabihf/bin:$PATH
make -j6
```

生成的 `demo` 由 Makefile 末尾自动拷贝到 `~/nfs/rootfs`，开发板通过 NFS 挂载根文件系统运行。

## 驱动与设备树

| 传感器 | 驱动形态 | 用户态接口 |
|---|---|---|
| DHT11 | `platform_driver` + `of_match` + `misc_register` | `/dev/dht11_misc`，一次读 5 字节 |
| BH1750 | `i2c_driver` + 设备树 + `misc_register` | `/dev/bh1750_misc`，一次读 1 个 `int`（mLux） |

BH1750 驱动读流程：`i2c_transfer` 读 2 字节原始值 → `raw * 1000 / 1200` 换算成毫勒克斯 →
`copy_to_user` 返回 4 字节；应用侧除以 1000 得到 lux。

```dts
&i2c1 {
    bh1750@23 {
        compatible = "rohm,bh1750fvi";
        reg = <0x23>;
        bh1750,measurement-mode   = <0x10>;
        bh1750,conversion-time-ms = <180>;
    };
};
```

## 已知限制

- 烟雾通道当前用随机数模拟，真实传感器接入未完成
- 消费端仍以 `usleep()` 轮询取消息；`wait_linkqueue()` 已实现但未切换（LVGL 线程阻塞会停 UI）

## 说明

仓库仅包含本项目的应用与移植代码；LVGL、paho.mqtt.c、sqlite3、OpenSSL 等第三方代码与预编译库未纳入。

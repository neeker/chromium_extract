base库说明文档
==========================

## 头文件及用途说明

| 头文件名 | 用途说明 | 其他说明 |
| -------------------------------- | ---------- | ------- |
| allocator/allocator_check.h | 内存分配管理 | 使用allocator库与去除默认malloc与new实现的libcmt库。 |
| allocator/allocator_extension.h | 内存分配扩展 | |
| at_exit.h | 退出清理器(AtExitManager) | 必须存在一个顶层对象。 |
| atomic_ref_count.h | 原子引用计数器 | |
| atomic_sequence_num.h | | |
| atomicops.h | | |
| atomicops_internals_atomicword_compat.h | | |
| atomicops_internals_portable.h | | |
| atomicops_internals_x86_msvc.h | | |
| auto_reset.h | | |
| barrier_closure.h | 栅栏回调工具 | 传入计数与回调函数返回一个计数回调函数（计数为0时才调用传入的回调函数） |
| base64.h | base64编码 | |
| base64url.h | base64URL编码 | |
| base_paths.h | 基本的路径定义键值 | |
| base_switches.h | base库中的开关常量 | |
| big_endian.h | 大端内存操作工具函数 | |
| bind.h | 函数绑定工具（引入闭包概念） | 类似std::bind |
| bind_helpers.h | 绑定函数指针传递辅助工具 | |
| bit_cast.h | 二进制级类型转换 | 仅限内存大小相等的类型 |
| bits.h | Log2Floor与Log2Ceiling | |
| build_time.h | base库编译时间 | |
| callback.h | 基本回调定义 | |
| callback_helpers.h | 回调工具函数 | |
| callback_list.h | 回调链表类实现 | 可用于插槽实现。 |
| cancelable_callback.h | 可撤销回调 | |
| command_line.h | 命令行处理与分析实现类 | |
| compiler_specific.h | 编译器特性相关宏定义 | |
| containers/adapters.h | 迭代器反转适配器 | stl容器操作 |
| containers/hash_tables.h | 哈希表 | |
| containers/linked_list.h | 链表 | |
| containers/mru_cache.h | MRU缓存 | |
| containers/scoped_ptr_hash_map.h | 智能指针哈希表 | |
| containers/small_map.h | 小哈希表实现（只读） | |
| containers/stack_container.h | 堆栈容器 | |
| cpu.h | CPU信息 | |
| deferred_sequenced_task_runner.h | 延时顺序执行任务运行器 | |
| environment.h | 系统环境变量操作类 | |
| feature_list.h | 特性列表 | ？ |
| file_version_info.h | 文件模块版本信息 | |
| files/file.h | 平台无关文件类 | |
| files/file_enumerator.h | 文件枚举类 | |
| files/file_path.h | 平台无关的文件路径类 | |
| files/file_path_watcher.h | 文件与路径监控 | |
| files/file_proxy.h | 文件异步读写代理类 | |
| files/file_util.h | 文件与路径工具函数 | |
| files/file_util_proxy.h | 文件异步处理代理工具函数 | |
| files/important_file_writer.h | 可靠的文件写入 | 程序奔溃也能写入 |
| files/memory_mapped_file.h | 内存映射文件类 | |
| files/scoped_file.h | 智能文件句柄 | |
| files/scoped_temp_dir.h | 智能临时文件 | |
| format_macros.h | sprintf与printf的格式宏定义 | |
| gtest_prod_util.h | gtest相关的宏定义 | 用于测试用例友类或函数快捷定义 |
| guid.h | GUID工具函数 | |
| hash.h | 哈希计算工具函数 | |
| i18n/base_i18n_switches.h | i18n开光常量定义 | |
| i18n/bidi_line_iterator.h | ICU的bidirectional封装器 | |
| i18n/break_iterator.h | | |
| i18n/case_conversion.h | i18n的大小写转换 | |
| i18n/char_iterator.h | UTF字符迭代器 | |
| i18n/file_util_icu.h | 文件路径字符串本地化工具函数 | |
| i18n/i18n_constants.h | i18n常量定义 | |
| i18n/icu_encoding_detection.h | ICU字符集编码判定 | |
| i18n/icu_string_conversions.h | ICU字符串转换 | |
| i18n/icu_util.h | ICU工具函数 | ICU初始化等 |
| i18n/message_formatter.h | 信息格式转换器 | |
| i18n/number_formatting.h | 数字本地化格式转换 | |
| i18n/rtl.h | 从右至左阅读方式相关工具函数 | |
| i18n/streaming_utf8_validator.h | 缓冲区UTF8校验工具 | |
| i18n/string_compare.h | i18n字符串比较 | |
| i18n/string_search.h | i18n字符串搜索函数 | |
| i18n/time_formatting.h | 时间本地化格式函数 | |
| i18n/timezone.h | 时区相关函数 | |
| id_map.h | ID对应对象指针映射表类 | |
| json/json_parser.h | JSON分析器 | |
| json/json_reader.h | JSON读取器 | |
| json/json_writer.h | JSON写入器 | |
| json/string_escape.h | JSON字符转椅工具函数 | |
| lazy_instance.h | 延时实例化单例类 | |
| location.h | 源码定位相关类与函数 | 用于调试跟踪 |
| logging.h | 日志处理函数 | 用于日志输出 |
| macros.h | 语言扩展的基础宏定义 |  |
| md5.h | MD5计算工具函数 | |
| memory/aligned_memory.h | 内存对齐分配器 | |
| memory/discardable_memory.h | 可废弃内存（虚类） | |
| memory/discardable_memory_allocator.h | 可废弃内存分配器（虚类） | |
| memory/discardable_shared_memory.h | 可废弃共享内存 | |
| memory/manual_constructor.h | 手动构造助手模板（使用内存对齐方式） | |
| memory/memory_pressure_listener.h | 内存压力监听器  | |
| memory/memory_pressure_monitor.h | 内存压力监控器 | 分不同的平台实现（chromos、mac、win） |
| memory/ptr_util.h | 智能指针封装 | 目前仅有有WrapUnique |
| memory/ref_counted.h | 引用计数实现基类 | |
| memory/ref_counted_delete_on_message_loop.h | 引用计数实现基类 | 在消息循环中释放 |
| memory/ref_counted_memory.h | 引用计数内存管理实现类 | 线程安全 |
| memory/scoped_ptr.h | 智能指针 | |
| memory/scoped_vector.h | 智能指针向量 | |
| memory/shared_memory.h | 共享内存操作类 | |
| memory/shared_memory_handle.h | 共享内存句柄类 | |
| memory/singleton.h | 单例 | |
| memory/weak_ptr.h | 弱引用智能指针 | |
| message_loop/message_loop.h | 消息循环处理器 | |
| message_loop/message_loop_task_runner.h | 消息循环任务运行器类 | |
| message_loop/message_pump.h | 消息泵虚类 | |
| message_loop/message_pump_libevent.h | libevent消息泵实现类 | |
| message_loop/message_pump_win.h | Windows消息泵实现类 | |
| move.h | CPP03 MoveCopy宏 | |
| native_library.h | 平台无关的本地共享链接库 | |
| nix/mime_util_xdg.h | 文件MineType获取工具函数 | |
| numerics/safe_conversions.h | 数值安全转换 | |
| numerics/safe_math.h | 数值工具函数与模板 | |
| observer_list.h | 观察者列表实现类 | |
| observer_list_threadsafe.h | 线程安全的观察者列表实现类 | |
| path_service.h | 路径服务工具类 | |
| pending_task.h | 待执行任务实现类 | |
| pickle.h | Pickle实现 | |
| posix/file_descriptor_shuffle.h | | |
| posix/global_descriptors.h | 全局描述符实现类 | |
| posix/safe_strerror.h | 错误值返回错误字串 | |
| posix/unix_domain_socket_linux.h | UnixDomain套接字 | |
| power_monitor/power_monitor.h | 电源监控工具函数 | |
| prefs/default_pref_store.h | 缺省实现的偏好存储器类 | |
| prefs/json_pref_store.h | JSON数据存储方式的偏好存储器类 | |
| prefs/persistent_pref_store.h | 持久化的偏好数据存储器类 | |
| prefs/overlay_user_pref_store.h | 可覆盖的用户偏好数据存储器类 | |
| prefs/pref_change_registrar.h | 偏好变更注册器类 | |
| prefs/pref_filter.h | 偏好数据过滤器 | |
| prefs/pref_member.h | 偏好成员管理类 | 变更监听 |
| prefs/pref_notifier.h | 偏好变更监听虚类 | |
| prefs/pref_observer.h | 偏好观察者类 | |
| prefs/pref_registry.h | 偏好注册表类 | |
| prefs/pref_service.h | 偏好服务实现类 | |
| prefs/pref_service_factory.h | 偏好服务工厂类 | |
| prefs/pref_store.h | 偏好存储虚类 | |
| process/kill.h | 杀进程工具函数 | |
| process/launch.h | 进程启动工具类及函数 | |
| process/memory.h | 进程相关内存操作工具函数 | 通过函数地址获得模块句柄（GetModuleFromAddress） |
| process/process.h | 平台无关的进程类实现 | |
| process/process_handle.h | 平台无关的进程句柄定义 | |
| process/process_info.h | 进程信息工具函数 | 仅有一个获得当前进程级别（WIN32） |
| process/process_iterator.h | 进程迭代器 | 枚举进程 |
| process/process_metrics.h | 进程运行统计信息类 | |
| rand_util.h | 随机数工具函数 | |
| run_loop.h | 循环执行器 | |
| scoped_generic.h | 通用范围内有效模板实现类 | |
| scoped_native_library.h | 智能本地共享库实现类 | |
| scoped_observer.h | 智能释放观察者实现类 | |
| sequence_checker.h | 顺序执行检查器实现类 | |
| sequenced_task_runner.h | 顺序任务执行器类 | |
| sequenced_task_runner_helpers.h | 顺序执行器辅助类与函数 | |
| sha1.h | SHA1函数 | |
| single_thread_task_runner.h | 单线程任务执行器 | |
| stl_util.h | STL辅助工具函数 | |
| strings/latin1_string_conversions.h | 拉丁字符串函数 | |
| strings/pattern.h | 正则表达式搜索函数 | |
| strings/safe_sprintf.h | 安全的字符串打印函数 | |
| strings/string16.h | 双字节UNICODE字符串类 | |
| strings/string_number_conversions.h | 字符串数字转换函数 | |
| strings/string_piece.h | 字符串增强类 | |
| strings/string_split.h | 字符串分割工具函数 | |
| strings/string_tokenizer.h | 简单的符号标记分割器 | |
| strings/string_util.h | 字符串工具函数 | 底层字符串函数的安全替换 |
| strings/stringprintf.h | 字符串打印函数 | |
| strings/sys_string_conversions.h | 系统字符集编码转换 | |
| strings/utf_offset_string_conversions.h | 字符串UTF转换位置计算函数 | |
| strings/utf_string_conversion_utils.h | 字符串UTF转换工具函数 | |
| strings/utf_string_conversions.h | 字符串UTF转换函数 | |
| supports_user_data.h | 用户关联数据存储类 | ？|
| sync_socket.h | 同步套接字实现 | Windows下通过管道（PIPE）实现 |
| synchronization/cancellation_flag.h | 取消标记类 | 线程安全 |
| synchronization/condition_variable.h | 条件变量类 | |
| synchronization/lock.h | 跨平台锁定义 | |
| synchronization/spin_wait.h | 自旋等待开关宏 | |
| synchronization/waitable_event.h | 可等待事件类 | |
| synchronization/waitable_event_watcher.h | 可等待时间监测类 | |
| sys_byteorder.h | 字节序工具函数及宏定义 | |
| sys_info.h | 系统信息类及函数 | CPU个数、物理内存、虚拟内存等。 |
| system_monitor/system_monitor.h | 设备监控类实现 | 目前仅实现音频与视频设备 |
| task/cancelable_task_tracker.h | 可取消任务跟踪类 | |
| task_runner.h | 任务执行器虚类 | |
| template_util.h | 模板工具类 | 是否可以转换模板类与是否某类型模板类 |
| third_party/dmg_fp/dmg_fp.h | 字符串与双精转换函数 | |
| third_party/dynamic_annotations/dynamic_annotations.h | 动态注释定义 | |
| threading/platform_thread.h | 平台无关线程类 | |
| threading/sequenced_worker_pool.h | 顺序工作线程池 | |
| threading/simple_thread.h | 简单线程实现类 | |
| threading/thread.h | 带消息处理机制的线程类 | |
| threading/thread_collision_warner.h | 线程冲突检查辅助宏 | DEBUG模式下有效 |
| threading/thread_id_name_manager.h | 线程ID与名称设置对应的管理类 | 单例实现，退出时不清理内存。 |
| threading/thread_local.h | 线程上下文存储实现类 | |
| threading/thread_restrictions.h | 线程限制条件操作类 | |
| threading/watchdog.h | 看门狗实现类 | |
| threading/worker_pool.h | 工作池工具类及函数 | 可能不会等待执行完毕就退出！ |
| time/clock.h | 时钟虚类 | |
| time/default_clock.h | 缺省时钟类 | |
| time/tick_clock.h | tick时钟虚类 | |
| time/default_tick_clock.h | 缺省的tick时钟类 | |
| time/time.h | 时间类 | |
| timer/elapsed_timer.h | 经过的时间定时器 | |
| timer/hi_res_timer_manager.h | 高精度定时器管理 | |
| timer/timer.h | 定时器类 | |
| tuple.h | 元组扩展类 | |
| value_conversions.h | 值转换工具函数 | |
| values.h | 基本值类型定义 | JSON相关 |
| version.h | 版本类实现 | |




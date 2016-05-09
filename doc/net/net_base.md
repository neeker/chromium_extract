net_base库说明文档
======================

## net库base

| 头文件名 | 用途说明 | 其他说明 |
| -------------------------------- | ---------- | ------- |
| address_family.h | IP地址家族工具函数 | 通过IP地址获得IP地址家族（IPV4,IPV6） |
| address_list.h | IP地址列表工具函数 | |
| auth.h | http认证凭据相关类 | AuthChallengeInfo、AuthCredentials、AuthData |
| backoff_entry.h | 网络退避指数相关类及函数 | ？ |
| backoff_entry_serializer.h | 上述类的序列化工具 | |
| chunked_upload_data_stream.h | 分块数据上传数据流类 | |
| completion_callback.h | 完成回调 | |
| connection_type_histograms.h | 更新连接类型直方图的工具函数 | |
| data_url.h | 数据URL解析工具函数 | 从URL中解析数据（当数据使用URL方式传递） |
| directory_lister.h | 异步实现的目录监听器 | |
| directory_listing.h | | 不需要 |
| elements_upload_data_stream.h | 非分块上传数据流类 | |
| escape.h | URL及HTML编码与解码工具函数 | |
| expiring_cache.h | 超期自动清理缓存类 | |
| external_estimate_provider.h | 外部参数估计提供器虚类 | 如获得RTT、下载带宽、上行带宽等数据 |
| file_stream.h | 异步实现的文件流类 | |
| filename_util.h | 本地文件名转URL的类及函数 | |
| hash_value.h | 哈希值转换类 | |
| host_mapping_rules.h | 主机映射规则 | 根据映射规则重置主机及端口 |
| host_port_pair.h | 主机及端口类 | |
| int128.h | 128位大整数实现 | |
| io_buffer.h | IO缓冲类实现 | |
| iovec.h | IOVEC结构封装 | 因为WIN32下没iovec结构定义。 |
| ip_address.h | IP地址封装类 |  |
| ip_address_number.h | IP地址字符类及工具函数 | |
| ip_endpoint.h | IP地址终端类 | 包括IP地址与端口 |
| ip_pattern.h | IP地址规则类 | 包括分析、匹配、ipv4判定等 |
| keygen_handler.h | 证书申请处理器 | 非Windows环境需要USE_NSS_CERTS或USE_OPENSSL_CERTS宏。 |
| linked_hash_map.h | 链接表哈希映射容器类 | |
| load_flags.h | 加载标记宏定义 | |
| load_states.h | 加载状态宏定义 | |
| load_timing_info.h | 加载时间信息 | |
| lookup_string_in_fixed_set.h | DAFSA查表工具函数 | 有向非循环字图（DAWG） |
| mime_sniffer.h | URL内容格式嗅探工具函数 | |
| mime_util.h | 文件内容格式工具函数 | |
| platform_mime_util.h | 平台包括的文件格式工具函数 | |
| net_errors.h | 网络错误宏定义及转字符串函数 | |
| net_module.h | 设置资源提供器回调 | |
| net_string_util.h | UTF字符串转换函数 | 依赖ICU实现 |
| net_util.h | 网络工具函数 | URL、主机名等常用工具函数 |
| network_activity_monitor.h | 网络活动监控类 | LEAKY实现（退出不清理） |
| network_change_notifier.h | 网络状态变更通知器 | |
| network_change_notifier_factory.h | 网络状态变更通知器实现工厂类 | |
| network_delegate.h | 网络委派器虚类 | |
| network_delegate_impl.h | 网络委派器实现类 | |
| layered_network_delegate.h | 分层网络委派器实现 | 用于扩展网络分派器实现。 |
| network_interfaces.h | 网络接口操作工具函数 | 如：获得接口列表、获取WifiSSID等 |
| network_quality_estimator.h | 网络质量评估类 | |
| port_util.h | 端口操作工具函数 | 如：端口有效性判定、是否已知端口等。 |
| prioritized_dispatcher.h | 根据优先级处理的排队任务处理分派器实现 | |
| proxy_delegate.h | 代理委派器虚类 | |
| sdch_dictionary.h | SDCH字典类 | |
| sdch_manager.h | SDCH管理器类 | |
| sdch_observer.h | SDCH观察者虚类 | |
| sockaddr_storage.h | sockaddr_storage的封装类 | |
| socket_performance_watcher.h | 套接字性能监测类 | 监测TCP的RTT（Round Trip Time）可用性 |
| socket_performance_watcher_factory.h | 套接字性能检测抽象工厂类 | |
| static_cookie_policy.h | 静态cookie策略类 | |
| upload_bytes_element_reader.h | 数据上传元素信息读取器类 | |
| upload_data_stream.h | 上传数据流类 | |
| upload_element_reader.h | 上传读取器类 | |
| upload_file_element_reader.h | 上传文件读取器类 | |
| upload_progress.h | 上传进度类 | |
| url_util.h | URL操作工具函数 | |
| winsock_init.h | WinSock初始化工具函数 | |
| winsock_util.h | WinSock工具函数 | |
| zap.h | 内存敏感数据清理工具函数 | |



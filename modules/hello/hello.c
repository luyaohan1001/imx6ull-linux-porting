/*
 * hello: a minimal out-of-tree kernel module for the i.MX6ULL port.
 *
 *   insmod hello.ko who=board     # prints "hello: hello, board!"
 *   cat /sys/module/hello/parameters/who
 *   rmmod hello                   # prints "hello: goodbye, board"
 */
#include <linux/init.h>
#include <linux/jiffies.h>
#include <linux/kernel.h>
#include <linux/module.h>

static char *who = "imx6ull";
module_param(who, charp, 0444);
MODULE_PARM_DESC(who, "Who to greet");

static int __init hello_init(void)
{
	pr_info("hello: hello, %s! (jiffies=%lu, HZ=%d)\n", who, jiffies, HZ);
	return 0;
}

static void __exit hello_exit(void)
{
	pr_info("hello: goodbye, %s\n", who);
}

module_init(hello_init);
module_exit(hello_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("luyaohan1001");
MODULE_DESCRIPTION("Minimal out-of-tree module for the i.MX6ULL port");

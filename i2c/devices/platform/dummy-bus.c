/*
 * i2c-dummy-bus.c: Dummy bus driver for educational purpose
 *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.
 *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Author: Yegnesh Iyer <yegnesh22@gmail.com>
 *
 */

#include <linux/slab.h>
#include <linux/platform_device.h>
#include <linux/module.h>

static struct platform_device *dummy_bus_device;

static int __init dummy_bus_device_add(void)
{
	dummy_bus_device = platform_device_alloc("i2c-dummy-bus", -1);
	return platform_device_add(dummy_bus_device);
}

static void __exit dummy_bus_device_remove(void)
{
	platform_device_unregister(dummy_bus_device);
}

module_init(dummy_bus_device_add);
module_exit(dummy_bus_device_remove);

MODULE_AUTHOR("Yegnesh S <yegnesh22@gmail.com>");
MODULE_DESCRIPTION("Dummy I2C bus device");
MODULE_LICENSE("GPL v2");

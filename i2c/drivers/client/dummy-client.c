/*
 * dummy-client.c: Dummy client driver for educational purpose
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
#include <linux/delay.h>
#include <linux/i2c.h>
#include <linux/debugfs.h>

#define MAX_REGS 256
#define DRIVER_NAME "i2c_dummy_device"

static struct i2c_client *dummy_dev_client;
static struct dentry *dummy_dev_debug_dir;

static int dummy_dev_fops_read(void *priv, u64 *val)
{
	unsigned char reg = (unsigned char)priv;

	*val = i2c_smbus_read_byte_data(dummy_dev_client, reg);

	return 0;
}

static int dummy_dev_fops_write(void *priv, u64 val)
{
	unsigned char reg = (unsigned char) priv;
	unsigned char data = (unsigned char) val;

	i2c_smbus_write_byte_data(dummy_dev_client, reg, data);

	return 0;
}

DEFINE_DEBUGFS_ATTRIBUTE(dummy_dev_fops, dummy_dev_fops_read, dummy_dev_fops_write, "%llu\n");

static int dummy_client_dev_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	char dirname[I2C_NAME_SIZE+4] = {0};
	char fname[3];
	int i;

	sprintf(dirname, "%s-%02x", client->name, client->addr);
	dummy_dev_client = client;

	dummy_dev_debug_dir = debugfs_create_dir(dirname, NULL);

	for (i = 0; i < MAX_REGS; i++) {
		sprintf(fname, "%02x", i);
		debugfs_create_file(fname, (S_IRUGO | S_IWUGO), dummy_dev_debug_dir, i, &dummy_dev_fops);
	}

	return 0;
}

static int dummy_client_dev_remove(struct i2c_client *client)
{
	debugfs_remove_recursive(dummy_dev_debug_dir);
	return 0;
}

static const struct i2c_device_id dummy_dev_id[] = {
	{ "i2c_dummy_device", 0 },
	{ "i2c_dummy_dev", 0 },
};

static struct i2c_driver dummy_client_driver = {
	.probe = dummy_client_dev_probe,
	.remove = dummy_client_dev_remove,
	.driver = {
		.name = DRIVER_NAME,
		.owner = THIS_MODULE,
	},
	.id_table = dummy_dev_id,
};

static int __init dummy_client_driver_init(void)
{
	return i2c_add_driver(&dummy_client_driver);
}

static void __exit dummy_client_driver_exit(void)
{
	i2c_del_driver(&dummy_client_driver);
}

module_init(dummy_client_driver_init);
module_exit(dummy_client_driver_exit);

MODULE_AUTHOR("Yegnesh S <yegnesh22@gmail.com>");
MODULE_DESCRIPTION("Dummy I2C device driver");
MODULE_LICENSE("GPL v2");


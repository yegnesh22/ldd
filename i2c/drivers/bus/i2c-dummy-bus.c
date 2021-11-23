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
#include <linux/delay.h>
#include <linux/i2c.h>
#include <linux/mutex.h>

#define DRIVER_NAME "i2c-dummy-bus"

#define MAX_DEV 256
#define MAX_REG 256

static unsigned char i2c_data_arr[MAX_DEV][MAX_REG] = {0};
static struct i2c_adapter *i2c_dummy_adapter;

static int i2c_dummy_smbus_xfer(struct i2c_adapter *adap,
				unsigned short addr,
				unsigned short flags,
				char read_write,
				unsigned char offset,
				int size,
				union i2c_smbus_data *data)
{
	int i;

	printk(KERN_INFO "fn:%s offset=%x size=%d read_write=%x ln:%d\n", __func__, offset, size, read_write, __LINE__);

	if (((offset + size) >= MAX_REG) || (addr >= MAX_DEV))
		return -EINVAL;

	if (read_write == I2C_SMBUS_WRITE) {
		printk(KERN_INFO "Writing %d bytes to address:%x\n", size, offset);
		for (i = 0; i < size; i++)
			i2c_data_arr[addr][offset+i]=data->block[i];
		return size;
	} else {
		printk(KERN_INFO "Reading %d bytes from address:%x\n", size, offset);
		for (i = 0; i < size; i++)
			data->block[i] = i2c_data_arr[addr][offset+i];
		return 0;
	}

	return -EINVAL;
}

static unsigned int i2c_dummy_bus_cap(struct i2c_adapter *adap)
{
	return (I2C_FUNC_I2C | I2C_FUNC_SMBUS_BYTE_DATA);
}

static const struct i2c_algorithm i2c_dummy_algo = {
	//.master_xfer = i2c_dummy_smbus_xfer,
	.functionality = i2c_dummy_bus_cap,
	.smbus_xfer = i2c_dummy_smbus_xfer,
};

static int i2c_dummy_bus_probe(struct platform_device *dev)
{
	struct i2c_adapter *adap;
	int ret;

	adap = kzalloc(sizeof(struct i2c_adapter), GFP_KERNEL);
	if (!adap)
		return -ENOMEM;

	adap->owner = THIS_MODULE;
	adap->class = I2C_CLASS_HWMON;
	adap->algo = &i2c_dummy_algo;
	strcpy(adap->name, "I2C Dummy Adapter");

	ret = i2c_add_adapter(adap);

	if (ret)
		printk(KERN_INFO "Error adding adapter\n");
	else {
	       printk(KERN_INFO "I2C bus %s added as bus number %d\n", adap->name, adap->nr);
	       i2c_dummy_adapter = adap;
	}

	return ret;	
}

static int i2c_dummy_bus_remove(struct platform_device *dev)
{
	kfree(i2c_dummy_adapter);
	return 0;
}

static struct platform_driver i2c_dummy_driver = {
	.probe = i2c_dummy_bus_probe,
	.remove = i2c_dummy_bus_remove,
	.driver = {
		.name = DRIVER_NAME,
		.owner = THIS_MODULE,
	},
};

module_platform_driver(i2c_dummy_driver);

MODULE_AUTHOR("Yegnesh S <yegnesh22@gmail.com>");
MODULE_DESCRIPTION("Dummy I2C bus driver");
MODULE_LICENSE("GPL v2");


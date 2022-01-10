/*
 * sfk.c - Softkey driver
 *
 * Software keypad driver for remote usage
 *
 *  Created on: 08-Jan-2022
 *  Author: yegnesh <yegnesh22@gmail.com>
 *
 */
#define DEBUG

#include <linux/platform_device.h>
#include <linux/module.h>
#include <linux/sysfs.h>
#include <linux/input.h>

#define CHAR_CODE_START 2
#define NUM_CODE_START ((CHAR_CODE_START + 26))

static unsigned short sfk_key_map[] = {
	KEY_ENTER, KEY_BACKSPACE,
	KEY_A, KEY_B, KEY_C, KEY_D, KEY_E,
	KEY_F, KEY_G, KEY_H, KEY_I, KEY_J,
	KEY_K, KEY_L, KEY_M, KEY_N, KEY_O,
	KEY_P, KEY_Q, KEY_R, KEY_S, KEY_T,
	KEY_U, KEY_V, KEY_W, KEY_X, KEY_Y,
	KEY_Z,
	KEY_0, KEY_1, KEY_2, KEY_3, KEY_4,
	KEY_5, KEY_6, KEY_7, KEY_8, KEY_9,
};

static int sfk_get_keycode(struct device* dev,
		const char* buf,
		unsigned short* keycode)
{
	int index = -1;

	dev_dbg(dev, "buf = %c ", buf[0]);
	if (buf[0] >= 'A' && buf[0] <= 'Z')
		index = CHAR_CODE_START + buf[0] - 'A';
	if (buf[0] >= 'a' && buf[0] <= 'z')
		index = CHAR_CODE_START + buf[0] - 'a';
	else if (buf[0] >= '0' && buf[0] <= '9')
		index = NUM_CODE_START + buf[0] - '0';
	else if (buf[0] == 13)
		index = 0;
	else if (buf[0] == 8)
		index = 1;
	dev_dbg(dev, "index = %d\n", index);

	*keycode = sfk_key_map[index];
	return index;
}

static ssize_t keyin_show(struct device* dev,
		struct device_attribute *attr,
		char *buf)
{
	*buf = 'X';
	return 1;
}

static ssize_t keyin_store(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t count)
{
	struct input_dev* input = dev_get_platdata(dev);
	unsigned short keycode;

	dev_dbg(dev, "fn:%s ln:%d\n", __func__, __LINE__);
	dev_dbg(dev, "input = %p\n", input);
	if (sfk_get_keycode(dev, buf, &keycode) > 0) {
		/* Post the pressed event */
		dev_dbg(dev, "key %c pressed posting keycode %d..!\n",
				buf[0], keycode);
		input_event(input, EV_KEY, keycode, 1);
		input_sync(input);

		/* Post the release event */
		input_event(input, EV_KEY, keycode, 0);
		input_sync(input);
	}
	dev_dbg(dev, "fn:%s ln:%d\n", __func__, __LINE__);

	return count;
}

static DEVICE_ATTR_RW(keyin);
static struct attribute * sfk_attrib [] = {
	&dev_attr_keyin.attr,
	NULL,
};
static struct attribute_group sfk_attribute_group = {
	.attrs = sfk_attrib,
};

static int sfk_probe(struct platform_device *pdev)
{
	struct device* dev = &pdev->dev;
	struct input_dev* input;
	int error, i;

	input = devm_input_allocate_device(dev);
	if (!input) {
		dev_err(dev, "Failed to allocate input device!!\n");
		return -ENOMEM;
	}
	dev->platform_data = input;
	input->name = pdev->name;
	input->phys = "sfk/input0";
	input->dev.parent = dev;
	input->open = NULL;
	input->close = NULL;

	input->id.bustype = BUS_HOST;
	input->id.vendor = 0x0001;
	input->id.product = 0x0001;
	input->id.version = 0x0100;

	input->keycode = sfk_key_map;
	input->keycodesize = sizeof(sfk_key_map[0]);
	input->keycodemax = ARRAY_SIZE(sfk_key_map);
	__set_bit(EV_REP, input->evbit);

	for (i = 0 ; i < ARRAY_SIZE(sfk_key_map); i++)
		input_set_capability(input, EV_KEY, sfk_key_map[i]);

	error = input_register_device(input);
	if (error) {
		dev_err(dev, "Unable to register input device err = %d !!\n",
				error);
		goto sfk_err_exit;
	}

	error = sysfs_create_group(&dev->kobj, &sfk_attribute_group);
	if (error) {
		dev_err(dev, "Unable to create sysfs attributes err = %d!!\n",
				error);
		goto sfk_err_exit;
	}

	return 0;

sfk_err_exit:
	input_free_device(input);
	return error;
}

static int sfk_remove(struct platform_device *pdev)
{
	struct device* dev = &pdev->dev;
	struct input_dev* input = dev_get_platdata(dev);

	sysfs_remove_group(&dev->kobj, &sfk_attribute_group);
	input_free_device(input);

	return 0;
}

static const struct platform_device_id sfk_ids[] = {
		{.name = "sfk"},
		{}
};
MODULE_DEVICE_TABLE(platform, sfk_ids);

static struct platform_driver sfk_driver = {
		.driver = {
				.name = "softkey-input",
		},
		.probe = sfk_probe,
		.remove = sfk_remove,
		.id_table = sfk_ids,
};

static struct platform_device* sfk_dev;

int sfk_device_add(void)
{
	sfk_dev = platform_device_alloc("sfk", -1);
	if (!sfk_dev) {
		pr_err("sfk: Error allocating device!!\n");
		return -1;
	}

	if (platform_device_add(sfk_dev)) {
		pr_err("sfk: Error adding platform device!!\n");
		return -1;
	}

	if (platform_driver_register(&sfk_driver)) {
		pr_err("sfk: Error registering platform driver!!\n");
		return -1;
	}

	return 0;
}

static void sfk_device_remove(void)
{
	platform_driver_unregister(&sfk_driver);
	platform_device_unregister(sfk_dev);
	return;
}

module_init(sfk_device_add);
module_exit(sfk_device_remove);

MODULE_DESCRIPTION("Softkeypad driver");
MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Yegnesh <yegnesh22@gmail.com");

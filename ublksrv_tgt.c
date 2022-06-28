#include "ublksrv.h"

static struct ublksrv_tgt_type *tgt_list[UBLKSRV_TGT_TYPE_MAX] = {};

int ublksrv_register_tgt_type(struct ublksrv_tgt_type *type)
{
	if (type->type < UBLKSRV_TGT_TYPE_MAX && !tgt_list[type->type]) {
		tgt_list[type->type] = type;
		return 0;
	}

	die("usbsrv: target %s/%d can't be registered\n",
			type->name, type->type);
	return -1;
}

int ublksrv_tgt_init(struct ublksrv_tgt_info *tgt, char *type, int argc, char
		*argv[])
{
	int i;

	if (type == NULL)
		return -1;

	for (i = 0; i < UBLKSRV_TGT_TYPE_MAX; i++) {
		const struct ublksrv_tgt_type  *ops = tgt_list[i];

		if (strcmp(ops->name, type))
			continue;

		if (!ops->init_tgt(tgt, i, argc, argv)) {
			tgt->ops = ops;
			return 0;
		}
	}

	return -1;
}

/*
 * Called in ublk daemon process context, and before creating per-queue
 * thread context
 */
int ublksrv_prepare_target(struct ublksrv_tgt_info *tgt, struct ublksrv_dev *dev)
{
	struct ublksrv_ctrl_dev *cdev = container_of(tgt,
			struct ublksrv_ctrl_dev, tgt);

	if (tgt->ops->prepare_target)
		return tgt->ops->prepare_target(tgt, dev);

	cdev->shm_offset += snprintf(cdev->shm_addr + cdev->shm_offset,
		UBLKSRV_SHM_SIZE - cdev->shm_offset,
		"target type: %s\n", tgt->ops->name);

	return 0;
}

void ublksrv_for_each_tgt_type(void (*handle_tgt_type)(unsigned idx,
			const struct ublksrv_tgt_type *type, void *data),
		void *data)
{
	int i;

	for (i = 0; i < UBLKSRV_TGT_TYPE_MAX; i++) {
		int len;

                const struct ublksrv_tgt_type  *type = tgt_list[i];

		if (!type)
			continue;
		handle_tgt_type(i, type, data);
	}
}

static inline void ublksrv_tgt_exit(struct ublksrv_tgt_info *tgt)
{
	int i;

	for (i = 1; i < tgt->nr_fds; i++)
		close(tgt->fds[i]);
}

void ublksrv_tgt_deinit(struct ublksrv_tgt_info *tgt, struct ublksrv_dev *dev)
{
	ublksrv_tgt_exit(tgt);

	if (tgt->ops->deinit_tgt)
		tgt->ops->deinit_tgt(tgt, dev);
}

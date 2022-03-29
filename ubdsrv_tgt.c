#include "ubdsrv.h"

static struct ubdsrv_tgt_type *tgt_list[UBDSRV_TGT_TYPE_MAX] = {};

int ubdsrv_register_tgt_type(struct ubdsrv_tgt_type *type)
{
	if (type->type < UBDSRV_TGT_TYPE_MAX && !tgt_list[type->type]) {
		tgt_list[type->type] = type;
		return 0;
	}

	die("usbsrv: target %s/%d can't be registered\n",
			type->name, type->type);
	return -1;
}

int ubdsrv_tgt_init(struct ubdsrv_tgt_info *tgt, char *type, int argc, char
		*argv[])
{
	int i;

	if (type == NULL)
		return -1;

	for (i = 0; i < UBDSRV_TGT_TYPE_MAX; i++) {
		const struct ubdsrv_tgt_type  *ops = tgt_list[i];

		if (strcmp(ops->name, type))
			continue;

		if (!ops->init_tgt(tgt, i, argc, argv)) {
			tgt->ops = ops;
			return 0;
		}
	}

	return -1;
}

/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (c) 2015, Linaro Limited
 */
#ifndef ELF_LOAD_H
#define ELF_LOAD_H

#include <sys/queue.h>
#include <types_ext.h>
#include <tee_api_types.h>

struct elf_load_state;

struct user_ta_store_handle;
struct user_ta_store_ops {
	/*
	 * Human-readable string to describe where the TA comes from.
	 * For debug purposes only.
	 */
	const char *description;
	/*
	 * Open a TA. Does not guarantee that the TA is valid or even exists.
	 */
	TEE_Result (*open)(const TEE_UUID *uuid,
			   struct user_ta_store_handle **h);
	/*
	 * Return the size of the unencrypted TA binary, that is: the TA
	 * header (struct ta_head) plus the ELF data.
	 */
	TEE_Result (*get_size)(const struct user_ta_store_handle *h,
			       size_t *size);
	/*
	 * Read the TA sequentially, from the start of the TA header (struct
	 * ta_head) up to the end of the ELF.
	 * The TEE core is expected to read *exactly* get_size() bytes in total
	 * unless an error occurs. Therefore, an implementation may rely on the
	 * condition (current offset == total size) to detect the last call to
	 * this function.
	 * @data: pointer to secure memory where the TA bytes should be copied.
	 * If @data == NULL and @len != 0, the function should just skip @len
	 * bytes.
	 */
	TEE_Result (*read)(struct user_ta_store_handle *h, void *data,
			   size_t len);
	/*
	 * Close a TA handle. Do nothing if @h == NULL.
	 */
	void (*close)(struct user_ta_store_handle *h);
	/*
	 * user_ta_store_ops instances are kept in a list ordered by priority.
	 * Higher priority instances are tried first when a TA is looked up.
	 * @priority lower value means higher priority
	 */
	SLIST_ENTRY(user_ta_store_ops) link;
	int priority;
};

TEE_Result elf_load_init(const struct user_ta_store_ops *ta_store,
			 struct user_ta_store_handle *ta_handle,
			 struct elf_load_state **state);
TEE_Result elf_load_head(struct elf_load_state *state, size_t head_size,
			void **head, size_t *vasize, bool *is_32bit);
TEE_Result elf_load_body(struct elf_load_state *state, vaddr_t vabase);
TEE_Result elf_load_get_next_segment(struct elf_load_state *state, size_t *idx,
			vaddr_t *vaddr, size_t *size, uint32_t *flags,
			uint32_t *type);
void elf_load_final(struct elf_load_state *state);

#endif /*ELF_LOAD_H*/

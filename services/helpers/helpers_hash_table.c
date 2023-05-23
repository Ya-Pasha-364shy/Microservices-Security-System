#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include "../../include/helpers/helpers_hash_table.h"
#include "../../include/helpers/helpers_common.h"

/*
 * @brief
 * Its "lose lose" hash-algorithm
*/
static hash_t __hash_key_generator(char * str)
{
	if (!str) return 0;
	unsigned int hash = 0;

	while (*str++)
		hash += *str;

	return hash % HASH_KEY_MAX_LEN;
}

static hash_item_t * __hash_item_create(char * value)
{
	if (!value) return NULL;

	size_t value_len = strlen(value);
	if (value_len > HASH_ITEM_MAX_SIZE_STR)
	{
		MSS_PRINT_INFO("Warning: length of given value is too big !");
		return NULL;
	}

	hash_item_t * hash_item = (hash_item_t *)malloc(sizeof(hash_item_t));
	memset(hash_item, 0, sizeof(hash_item_t));

	hash_item->value = (char *)malloc(sizeof(char) * value_len);
	memset(hash_item->value, 0, sizeof(char) * value_len);
	strncpy(hash_item->value, value, value_len);

	hash_t key = __hash_key_generator(value);
	if (key <= 1.1)
	{
		MSS_PRINT_INFO("Error: This key's value is too big !");
		return NULL;
	}
	else
	{
		hash_item->key = key;
		hash_item->next = NULL;
		hash_item->previous = NULL;

		return hash_item;
	}
}

hash_table_t * hash_table_create(hash_table_size_t size)
{
	if (size <= 0 || size > HASH_TABLE_MAX) return NULL;

	hash_table_t * this = (hash_table_t *)malloc(sizeof(hash_table_t));
	this->size = size;

	this->table = (hash_item_t **)calloc(this->size, sizeof(hash_item_t *));
	for (int i = 0; i < size; i++)
	{
		this->table[i] = NULL;
	}
	this->node = NULL;

	return this;
}

static hash_item_t * __hash_table_get_hi_by_key(hash_table_t * this, hash_t key)
{
	if (!this || !key) return NULL;

	return this->table[key];
}

static hash_item_t * __hash_table_get_hi_by_val(hash_table_t * this, char * value)
{
	if (!this || !value) return 0;

	return this->table[__hash_key_generator(value)];
}

hash_item_t * hash_table_search_item(hash_table_t * this, hash_t key, char * value)
{
	if (!this) return NULL;

	if (!key && value)
	{
		return __hash_table_get_hi_by_val(this, value);
	}
	else if (key && !value)
	{
		return __hash_table_get_hi_by_key(this, key);
	}

	return NULL;
}

unsigned int hash_table_insert_item(hash_table_t * this, char * value)
{
	// TODO: make handling of different return codes
	if (!this || !value) return 2;
	if (this->count == this->size)
	{
		MSS_PRINT_INFO("!!! Warning: Hash table is full !!!");
		return 1;
	}

	// this key from the "item" below
	hash_t key               = __hash_key_generator(value);
	if (!key) return 1;

	hash_item_t * item       = __hash_item_create(value);
	if (!item)
	{
		MSS_PRINT_INFO("!!! Warining: item is null !!!");
		return 1;
	}
	hash_item_t ** head      = &this->table[key];

	if (head && !(*head))
	{
		*head = item;
		if (NULL == this->node)
		{
			this->node = *head;
		}
		else
		{
			this->node->next = item;
			item->previous = this->node;
			this->node = item;
		}
		++this->count;
	}
	else if ((*head)->key == key)
	{
		char ** head_value = &(*head)->value;
		if (!strncmp(*head_value, item->value, strlen(*head_value)))
		{
			MSS_PRINT_INFO("Note: Full repetition of elements detected !");
		}
		else
		{
			MSS_PRINT_INFO("!!! Error: collision occurred. This item would be skipped !!!");
			return 2;
		}
	}

	return 0;
}

void hash_table_free(hash_table_t * this)
{
	for (int i = 0; i < this->size; i++)
	{
		hash_item_t * hash_item = this->table[i];
		if (hash_item != NULL)
		{
			free(hash_item->value);
			free(hash_item);
		}
	}

	free(this->table);
	free(this);
	return;
}

/* UNIT-tests */
#if 0
int test1()
{
	hash_table_t * HT = hash_table_create(HASH_TABLE_MAX_SIZE);
	hash_table_insert_item(HT, "/home/k1rch/value1");
	hash_table_insert_item(HT, "/home/k1rch/value2");
	hash_table_insert_item(HT, "/home/k1rch/value3");
	hash_table_insert_item(HT, "/home/k1rch/value4");
	hash_table_insert_item(HT, "/home/k1rch/value4");
	// COLLISION OCCURED HERE !
	hash_table_insert_item(HT, "biba");
	hash_table_insert_item(HT, "babi");

	hash_item_t * HI = hash_table_search_item(HT, 0, "/home/k1rch/value3");
	MSS_PRINT_INFO("[1] = %d:%s", HI->key, HI->value);
	hash_table_free(HT);
}
#endif
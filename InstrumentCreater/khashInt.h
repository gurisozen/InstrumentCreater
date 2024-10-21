/*
 * mykhashint.h
 *
 *  Created on: Jul 19, 2018
 *      Author: Osman
 */
#ifndef KHASH_INT_H  // Unique identifier for the file
#define KHASH_INT_H

#include <fstream>;
#include <stdlib.h>
#include <memory.h>

#include "khashCommon.h"

namespace Lib
{
	namespace Dictionary
	{
		template <typename ValueT>
		class CKhashInt
		{
			public:
				CKhashInt(int initialSize = 1024);
				~CKhashInt();

				int add(int key, ValueT value);
				void clear();
				void clearAndResize(int size);
				int get(int key, ValueT *output);
				void remove(int key);
				void set(int key, ValueT value);

				int getSize() const;
				int getMaxSize() const;

				void printDebugInfo(void (*printValueFnc)(ValueT*));

			private:
				int resize(int new_n_buckets);

			private:

				int n_buckets, size, n_occupied, upper_bound;
				int *flags;
				int *keys;
				ValueT *vals;

		};
	}
}

namespace Lib
{
	namespace Dictionary
	{
		template <typename ValueT>
		CKhashInt<ValueT>::CKhashInt(int initialSize)
		{
			n_buckets = 0;
			size = 0;
			n_occupied = 0;
			upper_bound = 0;
			flags = 0;
			keys = 0;
			vals = 0;

			resize(initialSize);
		}

		template <typename ValueT>
		CKhashInt<ValueT>::~CKhashInt()
		{
			free(keys);
			free(flags);
			free(vals);
		}

		template <typename ValueT>
		void CKhashInt<ValueT>::clear()
		{
			if (flags)
			{
				memset(flags, 0xaa, __int__ac_fsize(n_buckets) * sizeof(int));
				size = n_occupied = 0;
			}
		}

		template <typename ValueT>
		void CKhashInt<ValueT>::clearAndResize(int size)
		{
			clear();
			resize(size);
		}

		template <typename ValueT>
		int CKhashInt<ValueT>::getSize() const
		{
			return size;
		}

		template <typename ValueT>
		int CKhashInt<ValueT>::getMaxSize() const
		{
			return n_buckets;
		}

		/*! @function
		  @abstract     Retrieve a key from the hash table.
		  @param  name  Name of the hash table [symbol]
		  @param  h     Pointer to the hash table [khash_t(name)*]
		  @param  k     Key [type of keys]
		  @return       Iterator to the found element, or kh_end(h) if the element is absent [int]
		 */
		template <typename ValueT>
		int CKhashInt<ValueT>::get(int key, ValueT *output)
		{
			if (n_buckets)
			{
				
				int k, i, last, mask, step = 0;
				mask = n_buckets - 1;
				k = key;
				i = k & mask;
				last = i;
				while (
					!__int__ac_isempty(flags, i) &&
						(
						__int__ac_isdel(flags, i) ||
						keys[i] != key
						)
					)
				{
					i = (i + (++step)) & mask;
					if (i == last) return 0;
				}

				if (!__int__ac_iseither(flags, i))
				{
					*output = vals[i];
					return 1;
				}
				else
				{
					return 0;
				}
			}
			else
			{
				return 0;
			}
		}

		template <typename ValueT>
		void CKhashInt<ValueT>::set(int key, ValueT value)
		{
			if (n_buckets)
			{
				int k, i, last, mask, step = 0;
				mask = n_buckets - 1;
				k = key;
				i = k & mask;
				last = i;
				while (
					!__int__ac_isempty(flags, i) &&
						(
						__int__ac_isdel(flags, i) ||
						keys[i] != key
						)
					)
				{
					i = (i + (++step)) & mask;
					if (i == last) return ;
				}

				if (!__int__ac_iseither(flags, i))
				{
					vals[i] = value;
				}
				else
				{
					return ;
				}
			}
			else
			{
				return ;
			}
		}

		template <typename ValueT>
		void CKhashInt<ValueT>::remove(int key)
		{
			if (n_buckets)
			{
				int k, i, last, mask, step = 0;
				mask = n_buckets - 1;
				k = key;
				i = k & mask;
				last = i;
				while (
					!__int__ac_isempty(flags, i) &&
						(
						__int__ac_isdel(flags, i) ||
						keys[i] != key
						)
					)
				{
					i = (i + (++step)) & mask;
					if (i == last) return;
				}

				if (!__int__ac_iseither(flags, i))
				{
					__int__ac_set_isdel_true(flags, i);
					--size;
					return;
				}
				else
				{
					return;
				}
			}
			else
			{
				return;
			}
		}

		template <typename ValueT>
		int CKhashInt<ValueT>::resize(int new_n_buckets)
		{
			/* This function uses 0.25*n_buckets bytes of working space instead of [sizeof(key_t+val_t)+.25]*n_buckets. */
			int *new_flags = 0;
			int j = 1;
			{
				kroundup32(new_n_buckets);
				if (new_n_buckets < 4) new_n_buckets = 4;
				if (size >= (int)(new_n_buckets * __int__ac_HASH_UPPER + 0.5))
				{
					/* requested size is too small */
					j = 0;
				}
				else
				{
					/* hash table size to be changed (shrink or expand); rehash */
					new_flags = (int*)malloc(__int__ac_fsize(new_n_buckets) * sizeof(int));
					if (!new_flags) return -1;
					memset(new_flags, 0xaa, __int__ac_fsize(new_n_buckets) * sizeof(int));
					if (n_buckets < new_n_buckets)
					{
						/* expand */
						int *new_keys = (int*)realloc((void *)keys, new_n_buckets * sizeof(int));
						if (!new_keys)
						{
							free(new_flags);
							return -1;
						}
						keys = new_keys;

						ValueT *new_vals = (ValueT*)realloc((void *)vals, new_n_buckets * sizeof(ValueT));
						if (!new_vals)
						{
							free(new_flags);
							return -1;
						}
						vals = new_vals;

					}
					/* otherwise shrink */
				}
			}

			if (j)
			{
				/* rehashing is needed */
				for (j = 0; j != n_buckets; ++j)
				{
					if (__int__ac_iseither(flags, j) == 0)
					{
						int key = keys[j];
						ValueT val;
						int new_mask;
						new_mask = new_n_buckets - 1;
						val = vals[j];
						__int__ac_set_isdel_true(flags, j);
						while (1)
						{
							/* kick-out process; sort of like in Cuckoo hashing */
							int k, i, step = 0;
							k = key;
							i = k & new_mask;
							while (!__int__ac_isempty(new_flags, i)) i = (i + (++step)) & new_mask;
							__int__ac_set_isempty_false(new_flags, i);
							if (i < n_buckets && __int__ac_iseither(flags, i) == 0)
							{
								/* kick out the existing element */
								{ int tmp = keys[i]; keys[i] = key; key = tmp; }
								ValueT tmp = vals[i]; vals[i] = val; val = tmp;
								/* mark it as deleted in the old hash table */
								__int__ac_set_isdel_true(flags, i);
							}
							else
							{
								/* write the element and jump out of the loop */
								keys[i] = key;
								vals[i] = val;
								break;
							}
						}
					}
				}

				if (n_buckets > new_n_buckets)
				{
					/* shrink the hash table */
					keys = (int*)realloc((void *)keys, new_n_buckets * sizeof(int));
					vals = (ValueT*)realloc((void *)vals, new_n_buckets * sizeof(ValueT));
				}
				/* free the working space */
				free(flags);
				flags = new_flags;
				n_buckets = new_n_buckets;
				n_occupied = size;
				upper_bound = (int)(n_buckets * __int__ac_HASH_UPPER + 0.5);
			}
			return 0;
		}

		/*! @function
		  @abstract     Insert a key to the hash table.
		  @param  name  Name of the hash table [symbol]
		  @param  h     Pointer to the hash table [khash_t(name)*]
		  @param  k     Key [type of keys]
		  @param  r     Extra return code: -1 if the operation failed;
		                0 if the key is present in the hash table;
		                1 if the bucket is empty (never used); 2 if the element in
						the bucket has been deleted [int*]
		  @return       Iterator to the inserted element [int]
		 */
		template <typename ValueT>
		int CKhashInt<ValueT>::add(int key, ValueT value)
		{
			int x;
			
			if (n_occupied >= upper_bound)
			{
				/* update the hash table */
				if (n_buckets > (size<<1))
				{
					if (resize(n_buckets - 1) < 0)
					{
						/* clear "deleted" elements */
						return -1;
					}
				}
				else if (resize(n_buckets + 1) < 0)
				{
					/* expand the hash table */
					return -1;
				}
			}
			/* TODO: to implement automatically shrinking; resize() already support shrinking */
			{
				int k, i, site, last, mask = n_buckets - 1, step = 0;
				x = site = n_buckets; 
				k = key; 
				i = k & mask;
				if (__int__ac_isempty(flags, i))
				{
					x = i; /* for speed up */
				}
				else
				{
					last = i;
					while (
						!__int__ac_isempty(flags, i) &&
							(
							__int__ac_isdel(flags, i) ||
							keys[i] != key
							)
						)
					{
						if (__int__ac_isdel(flags, i)) site = i;
						i = (i + (++step)) & mask;
						if (i == last)
						{
							x = site;
							break;
						}
					}

					if (x == n_buckets)
					{
						if (__int__ac_isempty(flags, i) && site != n_buckets)
						{
							x = site;
						}
						else
						{
							x = i;
						}
					}
				}
			}

			if (__int__ac_isempty(flags, x))
			{
				/* not present at all */
				keys[x] = key;
				__int__ac_set_isboth_false(flags, x);
				++size; ++n_occupied;
			}
			else if (__int__ac_isdel(flags, x))
			{
				/* deleted */
				keys[x] = key;
				__int__ac_set_isboth_false(flags, x);
				++size;
			}
			else
			{
				/* Don't touch keys[x] if present and not deleted */
			}

			vals[x] = value;

			return 0;
		}

		/*template <typename ValueT>
		void CKhashInt<ValueT>::printDebugInfo(void (*printValueFnc)(ValueT*))
		{
			printf("Max size : %d \n", getMaxSize());
			printf("Size: %d \n", getSize());
			for (int idx = 0; idx < n_buckets; ++idx)
			{
				if (__int__ac_iseither(flags, idx))
				{
					printf("%-5d\n", idx);
				}
				else
				{
					printf("%-5d : [%-5d] : [", idx, keys[idx]);
					printValueFnc(vals + idx);
					printf("] \n");
				}
			}
			printf("\n");
		}*/
	}
}

#endif 

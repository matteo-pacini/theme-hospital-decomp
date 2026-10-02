extern "C" {

struct RT_TYPE_SIG;

struct ARRAY_STORAGE {
    unsigned count;
};

void *__wcpp_2_dtor_array_(void *array, unsigned count, RT_TYPE_SIG *sig);

ARRAY_STORAGE *__wcpp_2_dtor_array_store_(void *array, RT_TYPE_SIG *sig);

// __wcpp_2_dtor_array_store_: destroy a new[]-allocated array (count stored before it); returns the storage start, or NULL for a NULL array.
// FUNCTION: 0xeee94
ARRAY_STORAGE *__wcpp_2_dtor_array_store_(void *array, RT_TYPE_SIG *sig)
{
    ARRAY_STORAGE *store;

    if (array == 0) {
        store = 0;
    } else {
        store = (ARRAY_STORAGE *)array - 1;
        __wcpp_2_dtor_array_(array, store->count, sig);
    }
    return store;
}

}

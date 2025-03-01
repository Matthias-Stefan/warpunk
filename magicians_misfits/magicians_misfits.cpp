#include <warpunk.core/platform//platform.h>
#include <warpunk.core/input_system/input_system.h>
#include <cstdio>
#include <cassert>

#include <warpunk.core/container/dynarray.hpp>
#include <warpunk.core/container/dynqueue.hpp>

typedef struct _test_data_t
{
    b8 a;
    s16 b;
    s32 c;
    s64 d;
    f32 e;
    f64 f;
} test_data_t;

void game_test_dynarray()
{
    fprintf(stderr, "running dynarray test...\n");
    dynarray_t<test_data_t> array = dynarray_create<test_data_t>(5);
    assert(array.data != nullptr);
    assert(array.size == 5);
    assert(array.capacity == 0);
    
    dynarray_add(&array, { true, 100, 101, 102, 103.0f, 104.0 });
    dynarray_add(&array, { false, 200, 201, 202, 203.0f, 204.0 });
    dynarray_add(&array, { true, 300, 301, 302, 303.0f, 304.0 });
    dynarray_add(&array, { false, 400, 401, 402, 403.0f, 404.0 });
    dynarray_add(&array, { true, 500, 501, 502, 503.0f, 504.0 });
    assert(dynarray_front(&array).b == 100);
    assert(dynarray_back(&array).d == 502);

    dynarray_add(&array, { false, 600, 601, 602, 603.0f, 604.0 });
    assert(array.data != nullptr);
    assert(array.size == 10);
    assert(array.capacity == 6);

    dynarray_shrink_to_fit(&array);
    assert(array.size == 6);

    assert(dynarray_back(&array).d == 602);
    dynarray_remove(&array);
    assert(dynarray_back(&array).d == 502);
    assert(array.capacity == 5);

    assert(array.data[2].d == 302);
    dynarray_remove_at(&array, 2);
    assert(array.data[2].d == 402);
    assert(array.capacity == 4);

    dynarray_resize(&array, 500);
    assert(array.size == 500);

    dynarray_clear(&array);
    assert(array.data[0].b == 0);

    dynarray_destroy(&array);
    assert(array.data == nullptr);
    assert(array.size == 0);
    assert(array.capacity == 0);

    fprintf(stderr, "dynarray test passed!\n");
}

void game_test_dynqueue()
{
    fprintf(stderr, "running dynqueue test...\n");
    dynqueue_t<test_data_t> queue = dynqueue_create<test_data_t>(2);
    assert(queue.data != nullptr);
    assert(queue.size == 2);
    assert(queue.capacity == 0);
    assert(queue.head == 0);
    assert(queue.tail == 0);
    
    dynqueue_enqueue(&queue, { true, 100, 101, 102, 103.0f, 104.0 });
    assert(queue.data[0].b == 100);
    assert(queue.size == 2);
    assert(queue.capacity == 1);
    assert(queue.head == 0);
    assert(queue.tail == 1);
    
    dynqueue_enqueue(&queue, { false, 200, 201, 202, 203.0f, 204.0 });
    assert(queue.data[1].b == 200);
    assert(queue.size == 4);
    assert(queue.capacity == 2);
    assert(queue.head == 0);
    assert(queue.tail == 2);
    
    dynqueue_enqueue(&queue, { true, 300, 301, 302, 303.0f, 304.0 });
    assert(queue.data[2].b == 300);
    assert(queue.size == 4);
    assert(queue.capacity == 3);
    assert(queue.head == 0);
    assert(queue.tail == 3);
    
    test_data_t test_data;
    test_data = dynqueue_dequeue(&queue);
    assert(test_data.b == 100);
    assert(queue.size == 4);
    assert(queue.capacity == 2);
    assert(queue.head == 1);
    assert(queue.tail == 3);
    
    test_data = dynqueue_dequeue(&queue);
    assert(test_data.b == 200);
    assert(queue.size == 4);
    assert(queue.capacity == 1);
    assert(queue.head == 2);
    assert(queue.tail == 3);
   
    dynqueue_enqueue(&queue, { true, 500, 501, 502, 503.0f, 504.0 });
    assert(queue.data[3].b == 500);
    assert(queue.size == 4);
    assert(queue.capacity == 2);
    assert(queue.head == 2);
    assert(queue.tail == 0);
    
    dynqueue_enqueue(&queue, { true, 600, 601, 602, 603.0f, 604.0 });
    assert(queue.data[0].b == 600);
    assert(queue.size == 4);
    assert(queue.capacity == 3);
    assert(queue.head == 2);
    assert(queue.tail == 1);
    
    dynqueue_enqueue(&queue, { true, 700, 701, 702, 703.0f, 704.0 });
    assert(queue.data[3].b == 700);
    assert(queue.size == 8);
    assert(queue.capacity == 4);
    assert(queue.head == 0);
    assert(queue.tail == 4);
 
    dynqueue_clear(&queue);
    assert(queue.data[0].b == 0);
    assert(queue.size == 8);
    assert(queue.capacity == 0);
    assert(queue.head == 0);
    assert(queue.tail == 0);
 
    dynqueue_destroy(&queue);
    assert(queue.data == nullptr);
    assert(queue.size == 0);
    assert(queue.capacity == 0);
    assert(queue.head == 0);
    assert(queue.tail == 0);
 
    fprintf(stderr, "dynqueue test passed!\n");
}

void game_startup()
{
    game_test_dynarray();
    game_test_dynqueue();
}

void game_tick(double dt)
{
#if false 
    b8 is_key_a_up = input_system_is_key_up(KEY_A);
    b8 is_key_a_down = input_system_is_key_down(KEY_A);
    
    b8 was_key_a_up = input_system_was_key_up(KEY_A);
    b8 was_key_a_down = input_system_was_key_down(KEY_A);

    fprintf(stderr, "%s (is_up, is_down, was_up, was_down): %i, %i, %i, %i\n", input_system_keycode_str(KEY_A), is_key_a_up, is_key_a_down, was_key_a_up, was_key_a_down);
#endif
    
    s16 x_abs, y_abs, x_rel, y_rel;
    input_system_get_mouse_position(&x_abs, &y_abs, &x_rel, &y_rel);
    fprintf(stderr, "mouse (abs, rel): %i, %i, %i, %i\n", x_abs, y_abs, x_rel, y_rel);

    for (int i = 0; i < MOUSE_BUTTON_COUNT; ++i)
    {
        if (input_system_is_mouse_button_pressed((mouse_button_t)i))
        {
            fprintf(stderr, "%i pressed\n", i);
        }
    }

    s32 delta_abs, delta_rel;
    if (input_system_get_mouse_wheel_delta(&delta_abs, &delta_rel))
    {
        fprintf(stderr, "mouse_delta: %i, %i\n", delta_abs, delta_rel);
    }

    platform_window_info_t window_info = {};
    if (platform_get_window_info(&window_info))
    {
        if (window_info.platform_window_mode == FULLSCREEN)
        {
            fprintf(stderr, "FULLSCREEN\n");
        }
        else if (window_info.platform_window_mode == WINDOWED)
        {
            fprintf(stderr, "WINDOWED\n");
        }
    }

    if (platform_is_mouse_inside_window())
    {
        fprintf(stderr, "Inside\n");
    }

     [[maybe_unused]] bool _;
    if (input_system_is_key_down(KEY_8) && !input_system_was_key_down(KEY_8))
    {
        _ = platform_set_window_mode(FULLSCREEN);
    }
    if (input_system_is_key_down(KEY_9) && !input_system_was_key_down(KEY_9))
    {
        _ = platform_set_window_mode(WINDOWED);
    }
}


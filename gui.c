#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iup.h>
#include <iupdraw.h>
#include "gui.h"
#include "image_manage.h"
#include "image_process.h"

// --- Global Handles & State ---
Image *current_image = NULL;
Image *previous_image = NULL;
Image *original_image = NULL;
Ihandle *canvas = NULL;
Ihandle *status_label = NULL;
Ihandle *main_dlg = NULL;

// Dialog input handles
Ihandle *txt_crop_x = NULL, *txt_crop_y = NULL, *txt_crop_w = NULL, *txt_crop_h = NULL;
Ihandle *txt_brightness = NULL;
static int last_bright = 0; 

void save_state_for_undo() {
    if (!current_image) return;
    if (previous_image) destroy_image(previous_image);
    previous_image = copy_image(current_image);
}

// --- Canvas Callback ---
int canvas_action_cb(Ihandle *self) {
    int canvas_w = 0, canvas_h = 0;
    IupGetIntInt(self, "DRAWSIZE", &canvas_w, &canvas_h);

    IupDrawBegin(self);
    IupSetAttribute(self, "DRAWSTYLE", "FILL");
    IupSetAttribute(self, "DRAWCOLOR", "255 255 255");
    IupDrawRectangle(self, 0, 0, canvas_w, canvas_h);

    if (current_image && current_image->data) {
        int offset_x = (canvas_w - current_image->width) / 2;
        int offset_y = (canvas_h - current_image->height) / 2;
        if (offset_x < 0) offset_x = 0; 
        if (offset_y < 0) offset_y = 0;

        uint8_t *rgb_buffer = (uint8_t*)malloc(current_image->width * current_image->height * 3);
        if (rgb_buffer) {
            for (int i = 0; i < current_image->width * current_image->height; i++) {
                rgb_buffer[i*3+0] = current_image->data[i].r;
                rgb_buffer[i*3+1] = current_image->data[i].g;
                rgb_buffer[i*3+2] = current_image->data[i].b;
            }
            Ihandle *img_handle = IupImageRGB(current_image->width, current_image->height, rgb_buffer);
            IupSetHandle("temp_render_img", img_handle);
            IupDrawImage(self, "temp_render_img", offset_x, offset_y, current_image->width, current_image->height);
            IupDestroy(img_handle);
            free(rgb_buffer);
        }
    }
    IupDrawEnd(self);
    return IUP_DEFAULT;
}

// --- File Operations ---
int item_open_cb(Ihandle *self) {
    Ihandle *file_dlg = IupFileDlg();
    IupSetAttribute(file_dlg, "DIALOGTYPE", "OPEN");
    IupSetAttribute(file_dlg, "FILTER", "*.bmp");
    IupPopup(file_dlg, IUP_CENTER, IUP_CENTER);

    if (IupGetInt(file_dlg, "STATUS") != -1) {
        char *filename = IupGetAttribute(file_dlg, "VALUE");
        if (current_image) destroy_image(current_image);
        if (previous_image) { destroy_image(previous_image); previous_image = NULL; }
        if (original_image) { destroy_image(original_image); original_image = NULL; }
        
        current_image = open_image(filename);
        if (current_image) original_image = copy_image(current_image);
        
        IupSetStrAttribute(status_label, "TITLE", current_image ? "Status: Loaded image successfully." : "Status: Load failed.");
        IupUpdate(canvas);
    }
    IupDestroy(file_dlg);
    return IUP_DEFAULT;
}

int item_save_cb(Ihandle *self) {
    if (!current_image) { IupSetStrAttribute(status_label, "TITLE", "Status: Open an image first!"); return IUP_DEFAULT; }
    Ihandle *file_dlg = IupFileDlg();
    IupSetAttribute(file_dlg, "DIALOGTYPE", "SAVE");
    IupSetAttribute(file_dlg, "FILTER", "*.bmp");
    IupPopup(file_dlg, IUP_CENTER, IUP_CENTER);

    if (IupGetInt(file_dlg, "STATUS") != -1) {
        char *filename = IupGetAttribute(file_dlg, "VALUE");
        if (filename) {
            char final_path[512];
            strncpy(final_path, filename, sizeof(final_path) - 5);
            const char *ext = strrchr(final_path, '.');
            if (!ext || (strcmp(ext, ".bmp") != 0 && strcmp(ext, ".BMP") != 0)) strcat(final_path, ".bmp");
            
            if (save_image(current_image, final_path)) {
                IupSetStrAttribute(status_label, "TITLE", "Status: Saved successfully.");
            } else {
                IupSetStrAttribute(status_label, "TITLE", "Status: Save failed.");
            }
        }
    }
    IupDestroy(file_dlg);
    return IUP_DEFAULT;
}

int item_exit_cb(Ihandle *self) { return IUP_CLOSE; }

// --- Edit Operations ---
int item_undo_cb(Ihandle *self) {
    if (!previous_image) return IUP_DEFAULT;
    destroy_image(current_image);
    current_image = previous_image; previous_image = NULL;
    IupUpdate(canvas);
    return IUP_DEFAULT;
}

int item_revert_original_cb(Ihandle *self) {
    if (!original_image || !current_image) return IUP_DEFAULT;
    save_state_for_undo();
    destroy_image(current_image);
    current_image = copy_image(original_image);
    IupUpdate(canvas);
    return IUP_DEFAULT;
}

int item_gray_cb(Ihandle *self) {
    if (!current_image) return IUP_DEFAULT;
    save_state_for_undo();
    apply_grayscale(current_image);
    IupUpdate(canvas);
    return IUP_DEFAULT;
}

int item_invert_cb(Ihandle *self) {
    if (!current_image) return IUP_DEFAULT;
    save_state_for_undo(); invert_colors(current_image); IupUpdate(canvas); return IUP_DEFAULT;
}

int item_blur_cb(Ihandle *self) {
    if (!current_image) return IUP_DEFAULT;
    save_state_for_undo();
    Image *blurred = blur_image(current_image);
    if (blurred) { destroy_image(current_image); current_image = blurred; IupUpdate(canvas); }
    return IUP_DEFAULT;
}

int item_flip_h_cb(Ihandle *self) {
    if (!current_image) return IUP_DEFAULT;
    save_state_for_undo(); flip_horizontal(current_image); IupUpdate(canvas); return IUP_DEFAULT;
}

int item_flip_v_cb(Ihandle *self) {
    if (!current_image) return IUP_DEFAULT;
    save_state_for_undo(); flip_vertical(current_image); IupUpdate(canvas); return IUP_DEFAULT;
}

int item_rotate_90_cb(Ihandle *self) {
    if (!current_image) return IUP_DEFAULT;
    save_state_for_undo();
    Image *rotated = rotate_90_cw(current_image);
    if (rotated) { destroy_image(current_image); current_image = rotated; IupUpdate(canvas); }
    return IUP_DEFAULT;
}

// --- Brightness Dialog Callbacks ---
int btn_bright_apply_cb(Ihandle *self) {
    if (!current_image || !txt_brightness) return IUP_DEFAULT;
    int amount = atoi(IupGetAttribute(txt_brightness, "VALUE"));
    
    if (amount < -255 || amount > 255) {
        IupSetStrAttribute(status_label, "TITLE", "Status: Invalid brightness value (-255 to 255).");
        return IUP_DEFAULT;
    }
    
    last_bright = amount; 
    
    save_state_for_undo();
    adjust_brightness(current_image, amount);
    IupSetStrAttribute(status_label, "TITLE", "Status: Brightness adjusted.");
    IupUpdate(canvas);
    return IUP_CLOSE;
}

int btn_bright_cancel_cb(Ihandle *self) { return IUP_CLOSE; }

int item_brightness_cb(Ihandle *self) {
    if (!current_image) {
        IupSetStrAttribute(status_label, "TITLE", "Status: Open an image first!");
        return IUP_DEFAULT;
    }

    txt_brightness = IupText(NULL); 
    IupSetAttribute(txt_brightness, "VISIBLECOLUMNS", "6"); 
    
    char str_val[16];
    sprintf(str_val, "%d", last_bright);
    IupSetStrAttribute(txt_brightness, "VALUE", str_val);

    Ihandle *btn_apply = IupButton("Apply", NULL); 
    IupSetCallback(btn_apply, "ACTION", (Icallback)btn_bright_apply_cb);
    Ihandle *btn_cancel = IupButton("Cancel", NULL); 
    IupSetCallback(btn_cancel, "ACTION", (Icallback)btn_bright_cancel_cb);

    Ihandle *hbox_input = IupHbox(IupLabel("Amount (-255 to 255):"), txt_brightness, NULL);
    IupSetAttribute(hbox_input, "GAP", "10");
    IupSetAttribute(hbox_input, "ALIGNMENT", "ACENTER");

    Ihandle *hbox_buttons = IupHbox(IupFill(), btn_apply, btn_cancel, IupFill(), NULL);
    IupSetAttribute(hbox_buttons, "GAP", "12");

    Ihandle *vbox = IupVbox(hbox_input, hbox_buttons, NULL);
    IupSetAttribute(vbox, "MARGIN", "15x15");
    IupSetAttribute(vbox, "GAP", "15");

    Ihandle *bright_dlg = IupDialog(vbox);
    IupSetAttribute(bright_dlg, "TITLE", "Adjust Brightness");
    IupSetAttribute(bright_dlg, "SIZE", "200x120");
    IupSetAttribute(bright_dlg, "RESIZE", "NO");
    IupSetAttributeHandle(bright_dlg, "PARENTDIALOG", main_dlg);

    IupPopup(bright_dlg, IUP_CENTERPARENT, IUP_CENTERPARENT);
    IupDestroy(bright_dlg);
    return IUP_DEFAULT;
}

// --- Crop Dialog Callbacks ---
int btn_crop_apply_cb(Ihandle *self) {
    if (!current_image) return IUP_DEFAULT;
    int x = atoi(IupGetAttribute(txt_crop_x, "VALUE")), y = atoi(IupGetAttribute(txt_crop_y, "VALUE"));
    int w = atoi(IupGetAttribute(txt_crop_w, "VALUE")), h = atoi(IupGetAttribute(txt_crop_h, "VALUE"));

    if (w <= 0 || h <= 0) {
        IupSetStrAttribute(status_label, "TITLE", "Status: Invalid crop dimensions!");
        return IUP_DEFAULT;
    }

    save_state_for_undo();
    Image *cropped = crop_image(current_image, x, y, w, h);
    if (cropped) {
        destroy_image(current_image); current_image = cropped;
        IupSetStrAttribute(status_label, "TITLE", "Status: Cropped successfully.");
        IupUpdate(canvas);
    } else {
        IupSetStrAttribute(status_label, "TITLE", "Status: Crop region outside image bounds!");
    }
    return IUP_DEFAULT;
}

int btn_crop_cancel_cb(Ihandle *self) { return IUP_CLOSE; }

int item_crop_cb(Ihandle *self) {
    if (!current_image) return IUP_DEFAULT;
    txt_crop_x = IupText(NULL); IupSetAttribute(txt_crop_x, "VISIBLECOLUMNS", "6"); IupSetAttribute(txt_crop_x, "VALUE", "0");
    txt_crop_y = IupText(NULL); IupSetAttribute(txt_crop_y, "VISIBLECOLUMNS", "6"); IupSetAttribute(txt_crop_y, "VALUE", "0");
    
    char str_w[16], str_h[16];
    sprintf(str_w, "%d", current_image->width); sprintf(str_h, "%d", current_image->height);
    txt_crop_w = IupText(NULL); IupSetAttribute(txt_crop_w, "VISIBLECOLUMNS", "6"); IupSetStrAttribute(txt_crop_w, "VALUE", str_w);
    txt_crop_h = IupText(NULL); IupSetAttribute(txt_crop_h, "VISIBLECOLUMNS", "6"); IupSetStrAttribute(txt_crop_h, "VALUE", str_h);

    Ihandle *btn_apply = IupButton("Apply", NULL); IupSetCallback(btn_apply, "ACTION", (Icallback)btn_crop_apply_cb);
    Ihandle *btn_cancel = IupButton("Cancel", NULL); IupSetCallback(btn_cancel, "ACTION", (Icallback)btn_crop_cancel_cb);

    Ihandle *grid = IupGridBox(
        IupLabel("Start X (px):"), txt_crop_x, IupLabel("Start Y (px):"), txt_crop_y,
        IupLabel("Width (px):"), txt_crop_w, IupLabel("Height (px):"), txt_crop_h, NULL
    );
    IupSetAttribute(grid, "NUMDIV", "2"); IupSetAttribute(grid, "GAPLIN", "6"); IupSetAttribute(grid, "GAPCOL", "12");

    Ihandle *hbox_buttons = IupHbox(IupFill(), btn_apply, btn_cancel, IupFill(), NULL);
    IupSetAttribute(hbox_buttons, "GAP", "12");

    Ihandle *crop_dlg = IupDialog(IupVbox(grid, hbox_buttons, NULL));
    IupSetAttribute(crop_dlg, "TITLE", "Crop Parameters");
    IupSetAttribute(crop_dlg, "SIZE", "200x100");
    IupSetAttributeHandle(crop_dlg, "PARENTDIALOG", main_dlg);

    IupPopup(crop_dlg, IUP_CENTERPARENT, IUP_CENTERPARENT);
    IupDestroy(crop_dlg);
    return IUP_DEFAULT;
}

// --- View Menu Callbacks ---
int item_size_full_cb(Ihandle *self) {
    IupSetAttribute(main_dlg, "FULLSCREEN", "YES");
    IupSetStrAttribute(status_label, "TITLE", "Status: Fullscreen Mode.");
    return IUP_DEFAULT;
}

int item_size_medium_cb(Ihandle *self) {
    IupSetAttribute(main_dlg, "FULLSCREEN", "NO");
    IupSetAttribute(main_dlg, "RASTERSIZE", "800x600");
    IupRefresh(main_dlg);
    IupSetAttribute(main_dlg, "RASTERSIZE", NULL);
    IupSetStrAttribute(status_label, "TITLE", "Status: Resized Window (800x600).");
    return IUP_DEFAULT;
}

int item_size_small_cb(Ihandle *self) {
    IupSetAttribute(main_dlg, "FULLSCREEN", "NO");
    IupSetAttribute(main_dlg, "RASTERSIZE", "600x400");
    IupRefresh(main_dlg);
    IupSetAttribute(main_dlg, "RASTERSIZE", NULL);
    IupSetStrAttribute(status_label, "TITLE", "Status: Resized Window (600x400).");
    return IUP_DEFAULT;
}

// --- Main GUI Setup ---
void build_and_run_gui(int argc, char **argv) {
    IupOpen(&argc, &argv);

    // 1. File Menu
    Ihandle *item_open = IupItem("Open File...", NULL); IupSetCallback(item_open, "ACTION", (Icallback)item_open_cb);
    Ihandle *item_save = IupItem("Save Image", NULL); IupSetCallback(item_save, "ACTION", (Icallback)item_save_cb);
    Ihandle *item_exit = IupItem("Exit", NULL); IupSetCallback(item_exit, "ACTION", (Icallback)item_exit_cb);
    
    // 2. Edit Menu
    Ihandle *item_undo = IupItem("Undo", NULL); IupSetCallback(item_undo, "ACTION", (Icallback)item_undo_cb);
    Ihandle *item_revert_orig = IupItem("Revert to Original", NULL); IupSetCallback(item_revert_orig, "ACTION", (Icallback)item_revert_original_cb);
    Ihandle *item_gray = IupItem("Grayscale", NULL); IupSetCallback(item_gray, "ACTION", (Icallback)item_gray_cb);
    Ihandle *item_brightness = IupItem("Adjust Brightness...", NULL); IupSetCallback(item_brightness, "ACTION", (Icallback)item_brightness_cb);
    Ihandle *item_invert = IupItem("Image Inversion", NULL); IupSetCallback(item_invert, "ACTION", (Icallback)item_invert_cb);
    
    // Blur is here, but Sharpen is completely gone!
    Ihandle *item_blur = IupItem("Blur Image", NULL); IupSetCallback(item_blur, "ACTION", (Icallback)item_blur_cb);
    
    Ihandle *item_flip_h = IupItem("Horizontal Flip", NULL); IupSetCallback(item_flip_h, "ACTION", (Icallback)item_flip_h_cb);
    Ihandle *item_flip_v = IupItem("Vertical Flip", NULL); IupSetCallback(item_flip_v, "ACTION", (Icallback)item_flip_v_cb);
    Ihandle *item_rotate_90 = IupItem("Rotate 90 Degree", NULL); IupSetCallback(item_rotate_90, "ACTION", (Icallback)item_rotate_90_cb);
    Ihandle *item_crop = IupItem("Crop Image...", NULL); IupSetCallback(item_crop, "ACTION", (Icallback)item_crop_cb);

    // 3. View Menu
    Ihandle *item_full   = IupItem("Fullscreen Mode", NULL); IupSetCallback(item_full, "ACTION", (Icallback)item_size_full_cb);
    Ihandle *item_medium = IupItem("Medium Window (800x600)", NULL); IupSetCallback(item_medium, "ACTION", (Icallback)item_size_medium_cb);
    Ihandle *item_small  = IupItem("Small Window (600x400)", NULL); IupSetCallback(item_small, "ACTION", (Icallback)item_size_small_cb);

    // Submenus
    Ihandle *menu_file = IupMenu(item_open, item_save, IupSeparator(), item_exit, NULL);
    Ihandle *menu_edit = IupMenu(
        item_undo, item_revert_orig, IupSeparator(), 
        item_gray, item_brightness, item_invert, IupSeparator(), 
        item_blur, IupSeparator(), 
        item_flip_h, item_flip_v, item_rotate_90, IupSeparator(), 
        item_crop, NULL
    );
    Ihandle *menu_view = IupMenu(item_full, IupSeparator(), item_medium, item_small, NULL);
    
    // Main Top Menu
    Ihandle *main_menu = IupMenu(IupSubmenu("File", menu_file), IupSubmenu("Edit", menu_edit), IupSubmenu("View", menu_view), NULL);

    canvas = IupCanvas(NULL);
    IupSetAttribute(canvas, "EXPAND", "YES");
    IupSetAttribute(canvas, "RASTERSIZE", "800x600");
    IupSetCallback(canvas, "ACTION", (Icallback)canvas_action_cb);

    status_label = IupLabel("Status: Ready.");
    
    main_dlg = IupDialog(IupVbox(canvas, status_label, NULL));
    IupSetAttribute(main_dlg, "TITLE", "IMAGE MANIPULATION SOFTWARE");
    IupSetAttributeHandle(main_dlg, "MENU", main_menu);
    IupSetAttribute(main_dlg, "RESIZE", "YES");

    IupShowXY(main_dlg, IUP_CENTER, IUP_CENTER);
    IupMainLoop();

    if (current_image) destroy_image(current_image);
    if (previous_image) destroy_image(previous_image);
    if (original_image) destroy_image(original_image);
    IupClose();
}
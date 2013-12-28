/**
 * native pdflib implementation for Dart
 **/

#include "dartNativeHelpers.h"
#include "pdflib.h"

INIT_LIB(pdflib)

#define MAX_PDF 10
PDF *instances[MAX_PDF] = { 0, };

#define LIBPDF_TRY(pdf)  \
	PDF_TRY(pdf)
#define LIBPDF_CATCH(pdf) \
	PDF_CATCH(pdf) { \
		BEGIN_THROW("PDFlib exception"); \
		PDF_delete(pdf); \
		END_THROW \
	}

Dart_CObject *_currentResult = NULL;

#define SET_ERROR(_str) \
	_currentResult[0].type = Dart_CObject_kString; \
	_currentResult[0].value.as_string = (char *)(_str);

#define SET_RESULT(_typeName, _asType, _value) \
		_currentResult[1].type = _typeName; \
		_currentResult[1].value._asType = (_value);

#define SET_RESULT_STRING(_value) SET_RESULT(Dart_CObject_kString, as_string, _value)
#define SET_RESULT_INT(_value) SET_RESULT(Dart_CObject_kInt32, as_int32, _value)

#define GET_STRING(_var, _object)					\
	if (_object->type != Dart_CObject_kString) {	\
		SET_ERROR(#_var " should be a string");		\
		RETURN_ASYNC_FUNCTION;						\
	}												\
	char *_var = _object->value.as_string;

#define GET_INT(_var, _object)						\
	if (_object->type != Dart_CObject_kInt32) {		\
		SET_ERROR(#_var " should be an int");		\
		RETURN_ASYNC_FUNCTION;						\
	}												\
	int _var = _object->value.as_int32;

#define GET_DOUBLE(_var, _object)							\
	if (_object->type != Dart_CObject_kInt32				\
		&& _object->type != Dart_CObject_kDouble) {			\
		SET_ERROR(#_var " should be a double or an int");	\
		RETURN_ASYNC_FUNCTION;								\
	}														\
	double _var = _object->type == Dart_CObject_kInt32 ?	\
			_object->value.as_int32 : _object->value.as_double;

PDF *getPdf(Dart_CObject *o) {
	int i;

	if (o->type != Dart_CObject_kInt32) {
		SET_ERROR("handle required");
		return NULL;
	}
	i = o->value.as_int32;
	if (i >= MAX_PDF || instances[i] == 0) {
		SET_ERROR("bad handle");
		return NULL;
	}
	return instances[i];
}

int create(char *filename, char *options) {
	// find a place
	int i;
	PDF *pdf;
	for (i = 0; i < MAX_PDF; i++) {
		if (instances[i] == 0) {
			break;
		}
	}
	if (i >= MAX_PDF) {
		SET_ERROR("pdflib pool full");
		return -1;
	}
	pdf = PDF_new();
	if (pdf == (PDF *) 0) {
		SET_ERROR("can't create new PDF object");
		return -1;
	}
	instances[i] = pdf;

	PDF_TRY(pdf) {
#if PDFLIB_MAJORVERSION < 9
		PDF_set_parameter(pdf, "hypertextencoding", "host");
#else
		PDF_set_option(pdf, "errorpolicy=return");
#endif
		if (PDF_begin_document(pdf, filename, 0, options) == -1) {
			SET_ERROR(PDF_get_errmsg(pdf));
			PDF_delete(pdf);
			instances[i] = 0;
			return -1;
		}
	} PDF_CATCH(pdf) {
		SET_ERROR(PDF_get_errmsg(pdf));
		PDF_delete(pdf);
		instances[i] = 0;
		return -1;
	}

	return i;
}

int abort(Dart_CObject *handle) {
	int i;
	if (handle->type != Dart_CObject_kInt32) {
		SET_ERROR("handle required");
		return -1;
	}
	i = handle->value.as_int32;
	if (i >= MAX_PDF || instances[i] == 0) {
		SET_ERROR("bad handle");
		return -1;
	}
	PDF_delete(instances[i]);
	instances[i] = 0;
	return 0;
}

void getVersion(Dart_NativeArguments arguments) {
  Dart_EnterScope();
  Dart_Handle result = Dart_NewInteger(PDFLIB_MAJORVERSION);
  Dart_SetReturnValue(arguments, result);
  Dart_ExitScope();
}


/**
 * entry point for all functions
 */
BEGIN_ASYNC_FUNCTION(pdflibServicePort) {
//	Dart_CObject* message
//	Dart_CObject result
	Dart_CObject resultDetail[2];
	Dart_CObject *resultDetailPtr[2];
	resultDetail[0].type = Dart_CObject_kNull;
	resultDetailPtr[0] = &resultDetail[0];
	resultDetail[1].type = Dart_CObject_kNull;
	resultDetailPtr[1] = &resultDetail[1];
	result.type = Dart_CObject_kArray;
	result.value.as_array.length = 2;
	result.value.as_array.values = resultDetailPtr;
	_currentResult = resultDetail;

	if (argc < 1 || argv[0]->type != Dart_CObject_kString) {
		SET_ERROR("[ name, arguments...] expected");
		RETURN_ASYNC_FUNCTION;
	}
	char *name = argv[0]->value.as_string;
	argv++; argc--;

	if (strcmp("create", name) == 0) {
		if (argc != 2) {
			SET_ERROR("create : filename, options expected");
			RETURN_ASYNC_FUNCTION;
		}
		GET_STRING(fileName, argv[0])
		GET_STRING(options, argv[1])
		int handle = create(fileName, options);
		if (handle != -1) {
			SET_RESULT_INT(handle);
		}
		// else error should have been set

	} else if (strcmp("abort", name) == 0) {
		if (argc != 1) {
			SET_ERROR("abort : handle expected");
			RETURN_ASYNC_FUNCTION;
		}
		abort(argv[0]);

	} else if (strcmp("close", name) == 0) {
		if (argc != 1) {
			SET_ERROR("close: handle expected");
			RETURN_ASYNC_FUNCTION;
		}
		PDF *pdf = getPdf(argv[0]);

		PDF_TRY(pdf) {
			PDF_end_document(pdf, "");
		} PDF_CATCH(pdf) {
			SET_ERROR(PDF_get_errmsg(pdf));
		}
		abort(argv[0]);

	} else if (strcmp("setInfo", name) == 0) {
		if (argc != 3) {
			SET_ERROR("setInfo : handle, info, value expected");
			RETURN_ASYNC_FUNCTION;
		}
		PDF *pdf = getPdf(argv[0]);
		GET_STRING(info, argv[1]);
		GET_STRING(value, argv[2]);

		PDF_TRY(pdf) {
			PDF_set_info(pdf, info, value);
		} PDF_CATCH(pdf) {
			SET_ERROR(PDF_get_errmsg(pdf));
		}

	} else if (strcmp("beginPage", name) == 0) {
		if (argc != 4) {
			SET_ERROR("beginPage : handle, width, height, options expected");
			RETURN_ASYNC_FUNCTION;
		}
		PDF *pdf = getPdf(argv[0]);
		GET_DOUBLE(width, argv[1]);
		GET_DOUBLE(height, argv[2]);
		GET_STRING(options, argv[3]);

		PDF_TRY(pdf) {
			PDF_begin_page_ext(pdf, width, height, options);
		} PDF_CATCH(pdf) {
			SET_ERROR(PDF_get_errmsg(pdf));
		}

	} else if (strcmp("loadFont", name) == 0) {
		if (argc != 3) {
			SET_ERROR("loadFont : handle, fontName, options expected");
			RETURN_ASYNC_FUNCTION;
		}
		PDF *pdf = getPdf(argv[0]);
		GET_STRING(fontName, argv[1]);
		GET_STRING(options, argv[2]);

		PDF_TRY(pdf) {
			int fontHandle = PDF_load_font(pdf, fontName, 0, "host", options);
			SET_RESULT_INT(fontHandle);
		} PDF_CATCH(pdf) {
			SET_ERROR(PDF_get_errmsg(pdf));
		}

	} else if (strcmp("setFont", name) == 0) {
		if (argc != 3) {
			SET_ERROR("setFont : handle, fontHandle, size expected");
			RETURN_ASYNC_FUNCTION;
		}
		PDF *pdf = getPdf(argv[0]);
		GET_INT(fontHandle, argv[1]);
		GET_INT(fontSize, argv[2]);

		PDF_TRY(pdf) {
			PDF_setfont(pdf, fontHandle, fontSize);
		} PDF_CATCH(pdf) {
			SET_ERROR(PDF_get_errmsg(pdf));
		}

	} else if (strcmp("textTo", name) == 0) {
		if (argc != 5) {
			SET_ERROR("textTo : handle, text, x, y, options expected");
			RETURN_ASYNC_FUNCTION;
		}
		PDF *pdf = getPdf(argv[0]);
		if (argv[1]->type == Dart_CObject_kNull) {
			// text null => set pos
			GET_DOUBLE(x, argv[2]);
			GET_DOUBLE(y, argv[3]);

			PDF_TRY(pdf) {
				PDF_set_text_pos(pdf, x, y);
			} PDF_CATCH(pdf) {
				SET_ERROR(PDF_get_errmsg(pdf));
			}
		} else {
			GET_STRING(text, argv[1]);
printf("text '%s' -> %lu bytes", text, strlen(text));
			if (argv[2]->type == Dart_CObject_kNull) {
				// pos null => show / continue
				PDF_TRY(pdf) {
					if (text[0] == '\n') {
						PDF_continue_text(pdf, text + 1);
					} else {
						PDF_show(pdf, text);
					}
				} PDF_CATCH(pdf) {
					SET_ERROR(PDF_get_errmsg(pdf));
				}
			} else {
				// text + pos => fit_textline + consider options
				GET_DOUBLE(x, argv[2]);
				GET_DOUBLE(y, argv[3]);
				GET_STRING(options, argv[4]);
				PDF_TRY(pdf) {
					PDF_fit_textline(pdf, text, 0, x, y, options);
				} PDF_CATCH(pdf) {
					SET_ERROR(PDF_get_errmsg(pdf));
				}
			}
		}

	} else if (strcmp("endPage", name) == 0) {
		if (argc != 1) {
			SET_ERROR("endPage: handle expected");
			RETURN_ASYNC_FUNCTION;
		}
		PDF *pdf = getPdf(argv[0]);

		PDF_TRY(pdf) {
			PDF_end_page_ext(pdf, "");
		} PDF_CATCH(pdf) {
			SET_ERROR(PDF_get_errmsg(pdf));
		}

	} else if (strcmp("save", name) == 0) {
		if (argc != 1) {
			SET_ERROR("save: handle expected");
			RETURN_ASYNC_FUNCTION;
		}
		PDF *pdf = getPdf(argv[0]);

		PDF_TRY(pdf) {
			PDF_save(pdf);
		} PDF_CATCH(pdf) {
			SET_ERROR(PDF_get_errmsg(pdf));
		}

	} else if (strcmp("restore", name) == 0) {
		if (argc != 1) {
			SET_ERROR("restore: handle expected");
			RETURN_ASYNC_FUNCTION;
		}
		PDF *pdf = getPdf(argv[0]);

		PDF_TRY(pdf) {
			PDF_restore(pdf);
		} PDF_CATCH(pdf) {
			SET_ERROR(PDF_get_errmsg(pdf));
		}

	} else if (strcmp("translate", name) == 0) {
		if (argc != 3) {
			SET_ERROR("translate: handle, x, y expected");
			RETURN_ASYNC_FUNCTION;
		}
		PDF *pdf = getPdf(argv[0]);
		GET_DOUBLE(x, argv[1]);
		GET_DOUBLE(y, argv[2]);

		PDF_TRY(pdf) {
			PDF_translate(pdf, x, y);
		} PDF_CATCH(pdf) {
			SET_ERROR(PDF_get_errmsg(pdf));
		}

	} else if (strcmp("rotate", name) == 0) {
		if (argc != 2) {
			SET_ERROR("rotate: handle, angle expected");
			RETURN_ASYNC_FUNCTION;
		}
		PDF *pdf = getPdf(argv[0]);
		GET_DOUBLE(angle, argv[1]);

		PDF_TRY(pdf) {
			PDF_rotate(pdf, angle);
		} PDF_CATCH(pdf) {
			SET_ERROR(PDF_get_errmsg(pdf));
		}

	} else if (strcmp("setLineWidth", name) == 0) {
		if (argc != 2) {
			SET_ERROR("lineTo: handle, width expected");
			RETURN_ASYNC_FUNCTION;
		}
		PDF *pdf = getPdf(argv[0]);
		GET_DOUBLE(lineWidth, argv[1]);

		PDF_TRY(pdf) {
			PDF_setlinewidth(pdf, lineWidth);
		} PDF_CATCH(pdf) {
			SET_ERROR(PDF_get_errmsg(pdf));
		}

	} else if (strcmp("setColor", name) == 0) {
		if (argc != 5) {
			SET_ERROR("lineTo: handle, kind, r, g, b expected");
			RETURN_ASYNC_FUNCTION;
		}
		PDF *pdf = getPdf(argv[0]);
		GET_STRING(kind, argv[1]);
		GET_DOUBLE(r, argv[2]);
		GET_DOUBLE(g, argv[3]);
		GET_DOUBLE(b, argv[4]);

		PDF_TRY(pdf) {
			PDF_setcolor(pdf, kind, "rgb", r, g, b, 0);
		} PDF_CATCH(pdf) {
			SET_ERROR(PDF_get_errmsg(pdf));
		}

	} else if (strcmp("moveTo", name) == 0) {
		if (argc != 3) {
			SET_ERROR("moveTo: handle, x, y expected");
			RETURN_ASYNC_FUNCTION;
		}
		PDF *pdf = getPdf(argv[0]);
		GET_DOUBLE(x, argv[1]);
		GET_DOUBLE(y, argv[2]);

		PDF_TRY(pdf) {
			PDF_moveto(pdf, x, y);
		} PDF_CATCH(pdf) {
			SET_ERROR(PDF_get_errmsg(pdf));
		}

	} else if (strcmp("lineTo", name) == 0) {
		if (argc != 3) {
			SET_ERROR("lineTo: handle, x, y expected");
			RETURN_ASYNC_FUNCTION;
		}
		PDF *pdf = getPdf(argv[0]);
		GET_DOUBLE(x, argv[1]);
		GET_DOUBLE(y, argv[2]);

		PDF_TRY(pdf) {
			PDF_lineto(pdf, x, y);
		} PDF_CATCH(pdf) {
			SET_ERROR(PDF_get_errmsg(pdf));
		}

	} else if (strcmp("curveTo", name) == 0) {
		if (argc != 7) {
			SET_ERROR("lineTo: handle, x1, y1, x2, y2, x3, y3 expected");
			RETURN_ASYNC_FUNCTION;
		}
		PDF *pdf = getPdf(argv[0]);
		GET_DOUBLE(x1, argv[1]);
		GET_DOUBLE(y1, argv[2]);
		GET_DOUBLE(x2, argv[3]);
		GET_DOUBLE(y2, argv[4]);
		GET_DOUBLE(x3, argv[5]);
		GET_DOUBLE(y3, argv[6]);

		PDF_TRY(pdf) {
			PDF_curveto(pdf, x1, y1, x2, y2, x3, y3);
		} PDF_CATCH(pdf) {
			SET_ERROR(PDF_get_errmsg(pdf));
		}

// TODO : PDF_circle(PDF *p, double x, double y, double r);
// TODO : PDF_arc(PDF *p, double x, double y, double r, double alpha, double beta);/* counterclockwise  */
// TODO : PDF_arcn(PDF *p, double x, double y, double r, double alpha, double beta);/* clockwise */

	} else if (strcmp("terminatePath", name) == 0) {
		if (argc != 2) {
			SET_ERROR("terminatePath: handle, what expected");
			RETURN_ASYNC_FUNCTION;
		}
		PDF *pdf = getPdf(argv[0]);
		GET_INT(what, argv[1]);

		PDF_TRY(pdf) {
			if (what & 4) {
				PDF_closepath(pdf);
			}
			switch(what & 3) {
			case 1:
				PDF_stroke(pdf);
				break;
			case 2:
				PDF_fill(pdf);
				break;
			case 3:
				PDF_fill_stroke(pdf);
				break;
			}
		} PDF_CATCH(pdf) {
			SET_ERROR(PDF_get_errmsg(pdf));
		}

	} else if (strcmp("loadImage", name) == 0) {
		if (argc != 3) {
			SET_ERROR("loadImage : handle, imagePath, options expected");
			RETURN_ASYNC_FUNCTION;
		}
		PDF *pdf = getPdf(argv[0]);
		GET_STRING(imagePath, argv[1]);
		GET_STRING(options, argv[2]);

		PDF_TRY(pdf) {
			int imageHandle = PDF_load_image(pdf, "auto", imagePath, 0, options);
			if (imageHandle < 0) {
				SET_ERROR(PDF_get_errmsg(pdf));
			} else {
				SET_RESULT_INT(imageHandle);
			}
		} PDF_CATCH(pdf) {
			SET_ERROR(PDF_get_errmsg(pdf));
		}

	} else if (strcmp("imageTo", name) == 0) {
		if (argc != 5) {
			SET_ERROR("imageTo: handle, imageHandle, x, y, options expected");
			RETURN_ASYNC_FUNCTION;
		}
		PDF *pdf = getPdf(argv[0]);
		GET_INT(imageHandle, argv[1]);
		GET_DOUBLE(x, argv[2]);
		GET_DOUBLE(y, argv[3]);
		GET_STRING(options, argv[4]);

		PDF_TRY(pdf) {
			PDF_fit_image(pdf, imageHandle, x, y, options);
		} PDF_CATCH(pdf) {
			SET_ERROR(PDF_get_errmsg(pdf));
		}

	} else if (strcmp("setOption", name) == 0) {
		if (argc != 3) {
			SET_ERROR("setOption : handle, key, value expected");
			RETURN_ASYNC_FUNCTION;
		}
		PDF *pdf = getPdf(argv[0]);
		GET_STRING(key, argv[1]);
		GET_STRING(value, argv[2]);

		PDF_TRY(pdf) {
#if PDFLIB_MAJORVERSION < 9
			PDF_set_parameter(pdf, key, value);
#else
			char opt[200];
			snprintf(opt, 200, "%s=%s", key, value);
			PDF_set_option(pdf, opt);
#endif
		} PDF_CATCH(pdf) {
			SET_ERROR(PDF_get_errmsg(pdf));
		}

	} else {
		SET_ERROR("unknown command");
		RETURN_ASYNC_FUNCTION;
	}
	_currentResult = NULL;
} END_ASYNC_FUNCTION(newPdfServicePort)

BEGIN_NAMES
ADD_NAME(getVersion)
ADD_NAME(pdflibServicePort)
END_NAMES

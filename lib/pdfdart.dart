library pdflib;

import 'dart:async';
import 'dart:isolate';
import 'dart-ext:pdflib';

class PDF {
	int handle;
	Future f;

	/**
	 * default pdf measure unit is "point" (1/72 inch). This table contains
	 * some ratio to use other units to pass to {@see setUnit} method
	 */
	static Map<String,double> unitName = {
		"point": 1,
		"millimeter": 0.3528,
		"inch": 0.0139
	};

	/**
	 * some standard paper sizes to pass to {@see beginPage} method
	 */
	static Map<String,List<double>> pageSize = {
		"a0" : [ 2380.0, 3368.0 ],
		"a1" : [1684.0, 2380.0 ],
		"a2" : [1190.0, 1684.0 ],
		"a3" : [842.0, 1190.0 ],
		"a4" : [595.0, 842.0 ],
		"a5" : [421.0, 595.0 ],
		"a6" : [297.0, 421.0 ],
		"b5" : [501.0, 709.0 ],
		"letter" : [612.0, 792.0 ],
		"legal"  : [612.0, 1008.0 ],
		"ledger" : [1224.0, 792.0 ],
		"11x17"  : [792.0, 1224.0 ]
	};

	/**
	 * constants to pass to {@see terminatePath} method
	 */
	static final int STROKE = 1;
	static final int FILL = 2;
	static final int FILL_STROKE = 3;
	static final int CLOSE = 4;

	double _unit = unitName["point"];

	/**
	 * prepare new PDF document
	 */
	PDF(String filename, [ String options = "" ]) {
		ReceivePort response = new ReceivePort();

		_servicePort.send([response.sendPort, "create", filename, options]);
		f = response.first.then((List result) {
			if (result[0] == null) {
				handle = result[1];
			} else {
				throw new Exception("failed : " + result[0]);
			}
		}).catchError((e){
			print("exception on create : ${e}");
		});
	}

	/**
	 * many PDF methods are asynchonous. This method runs it's callback when
	 * all pending actions are achieved.
	 */
	Future then(onValue, {onError: null}) {
		return f.then(onValue, onError: onError);
	}

	/**
	 * Change current measure unit. All method calls made after will divide
	 * lenght arguments by this value before passing them to pdflib
	 * EXCEPTED for pageSize argument in {@see beginPath}
	 */
	void setUnit(num ratio) {
		f = f.then((_) {
			_unit = ratio;
		});
	}

	/**
	 * abort document. PDF object must not be used after.
	 */
	void abort() {
		f = f.then((_) {
			ReceivePort response = new ReceivePort();
			_servicePort.send([response.sendPort, "abort", handle]);
			return response.first;
		}).then((List result) {
			if (result[0] != null) {
				throw new Exception("failed : " + result[0]);
			}
			f = null;
		}).catchError((e){
			print("exception on abort : ${e}");
		});
	}

	/**
	 * save document and free structure. PDF object must not be used after.
	 */
	void close() {
		f = f.then((_) {
			ReceivePort response = new ReceivePort();
			_servicePort.send([response.sendPort, "close", handle]);
			return response.first;
		}).then((List result) {
			if (result[0] != null) {
				throw new Exception("failed : " + result[0]);
			}
			f = null;
		}).catchError((e){
			print("exception on close : ${e}");
		});
	}

	void setInfo(String info, String value) {
		_doCall("setInfo", [info, value]);
	}

	/**
	 * starts a new page, with given size and optional options
	 */
	void beginPage(List<double> size, [ String options = "" ]) {
		_doCall("beginPage", [size[0], size[1], options]);
	}

	/**
	 * Load and prepare a font
	 * @return [Future<int>] whrn font is loaded, int is a handle to refer to in {@see setFont}
	 */
	Future<int> loadFont(String fontName, [ String options = "" ]) {
		return f.then((_) {
			ReceivePort response = new ReceivePort();
			_servicePort.send([response.sendPort, "loadFont", handle, fontName, options]);
			return response.first;
		}).then((List result) {
			if (result[0] != null) {
				throw new Exception("failed : " + result[0]);
			} else {
				return result[1];
			}
		}).catchError((e){
			print("exception on loadFont : ${e}");
		});
	}

	void setFont(Future<int> font, num size) {
		f = Future.wait([font, f]).then((List responses) {
			int fontHandle = responses[0];
			ReceivePort response = new ReceivePort();
			_servicePort.send([response.sendPort, "setFont", handle,
				fontHandle, (size / _unit).round()]);
			return response.first;
		}).then((List result) {
			if (result[0] != null) {
				throw new Exception("failed : " + result[0]);
			}
		}).catchError((e){
			print("exception on setFont : ${e}");
		});
	}

	/**
	 * Draw text with current font and stroke style.
	 * If pos is not given, text starts at current position, or
	 * if text starts with '\n', under last text line
	 */
	void textTo(String text, [num x= null, num y= null, options= ""]) {
		if ((text == null || x == null) && options != "") {
			throw new Exception("options used only if text AND pos are specified");
		}
		_doCall("textTo", () {
			return [text, (x != null) ? x / _unit : x, (y != null) ? y / _unit : y, options];
		});
	}

	/**
	 * terminates current page
	 */
	void endPage() {
		_doCall("endPage");
	}

	/**
	 * save current styles and position
	 */
	void save() {
		_doCall("save");
	}

	/**
	 * restore last saved styles and position
	 */
	void restore() {
		_doCall("restore");
	}

	/**
	 * Change current coordinate system
	 */
	void translate(num x, num y) {
		_doCall("translate", () { return [x / _unit, y / _unit]; });
	}

	/**
	 * Change current coordinate system
	 */
	void rotate(num angle) {
		_doCall("rotate", [angle]);
	}

	/**
	 * Set current stroke width
	 */
	void setLineWidth(num width) {
		_doCall("setLineWidth", () { return [width / _unit]; });
	}

	/**
	 * Set current stroke or fill color.
	 * Color components must be values between 0 and 1
	 *
	 */
	void setColor(String kind, num r, num g, num b) {
		_doCall("setColor", [kind, r, g, b]);
	}

	/**
	 * Set current position
	 */
	void moveTo(num x, num y) {
		_doCall("moveTo", () { return [x / _unit, y / _unit]; });
	}

	void lineTo(num x, num y) {
		_doCall("lineTo", () { return [x / _unit, y / _unit]; });
	}

	void curveTo(num x1, num y1, num x2, num y2, num x3, num y3) {
		_doCall("curveTo", () {
			return [
				x1 / _unit, y1 / _unit,
				x2 / _unit, y2 / _unit,
				x3 / _unit, y3 / _unit
			];
		});
	}

	void circle(num x, num y, num r) {
		_doCall("circle", () {
			return [x / _unit, y / _unit, r / _unit];
		});
	}

	void arc(num x, num y, num r, num alpha, num beta, [ bool clockwise = false ]) {
		_doCall("arc", () {
			return [x / _unit, y / _unit, r / _unit, alpha, beta, clockwise];
		});
	}

	Future loadImage(String imagePath, [ String options = "" ]) {
		return f.then((_) {
			ReceivePort response = new ReceivePort();
			_servicePort.send([response.sendPort, "loadImage", handle, imagePath, options]);
			return response.first;
		}).then((List result) {
			if (result[0] != null) {
				throw new Exception("failed : ${result[0]}");
			} else {
				return result[1];
			}
		}).catchError((e){
			print("exception on loadImage : ${e}");
		});
	}

	void imageTo(Future<int> image, num x, num y,  [ String options = "" ]) {
		f = Future.wait([image, f]).then((List responses) {
			int imageHandle = responses[0];
			ReceivePort response = new ReceivePort();
			_servicePort.send([
				response.sendPort, "imageTo", handle,
				imageHandle, x / _unit, y / _unit, options
			]);
			return response.first;
		}).then((List result) {
			if (result[0] != null) {
				throw new Exception("failed : " + result[0]);
			}
		}).catchError((e){
			print("exception on imageTo : ${e}");
		});
	}

	void setOption(String key, String value) {
		_doCall("setOption", [key, value]);
	}

	void terminatePath(int what) {
		_doCall("terminatePath", [what]);
	}

	void _doCall(String command, [ args = null ]) {
		f = f.then((_) {
			if (args is Function) {
				args = (args as Function)();
			}
//			print("calling ${command}");
			ReceivePort response = new ReceivePort();
			List callArgs = [response.sendPort, command, handle ];
			if (args != null) {
				callArgs.addAll(args);
			}
			_servicePort.send(callArgs);
			return response.first;
 		}).then((List result) {
			if (result[0] != null) {
				throw new Exception("failed : " + result[0]);
			}
		}).catchError((e){
			print("exception on ${command} : ${e}");
		});
	}

	static SendPort _port;
	SendPort get _servicePort {
		if (_port == null) {
			_port = _newServicePort();
		}
		return _port;
	}
	static SendPort _newServicePort() native "pdflibServicePort";
	static num getVersion() native "getVersion";
}

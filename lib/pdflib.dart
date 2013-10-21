library pdflib;

import 'dart:async';
import 'dart:isolate';
import 'dart-ext:pdflib';

class PDF {
	int handle;
	Future f;

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

	static final int STROKE = 1;
	static final int FILL = 2;
	static final int FILL_STROKE = 3;
	static final int CLOSE = 4;

	/**
	 * prepare new PDF document
	 */
	PDF(String filename, [ String options = "" ]) {
		f = _servicePort.call(["create", filename, options]).then((List result) {
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
	 * abort document
	 */
	void delete() {
		f = f.then((_) {
			return _servicePort.call(["delete", handle]);
		}).then((List result) {
			if (result[0] != null) {
				throw new Exception("failed : " + result[0]);
			}
			f = null;
		}).catchError((e){
			print("exception on delete : ${e}");
		});
	}

	/**
	 * save document and free structure
	 */
	void close() {
		f = f.then((_) {
			return _servicePort.call(["close", handle]);
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

	void beginPage(List<double> size, [ String options = "" ]) {
		_doCall("beginPage", [size[0], size[1], options]);
	}

	Future loadFont(String fontName, [ String options = "" ]) {
		return f.then((_) {
			return _servicePort.call(["loadFont", handle, fontName, options]);
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

	void setFont(Future<int> font, int size) {
		f = Future.wait([font, f]).then((List responses) {
			int fontHandle = responses[0];
			return _servicePort.call(["setFont", handle, fontHandle, size]);
		}).then((List result) {
			if (result[0] != null) {
				throw new Exception("failed : " + result[0]);
			}
		}).catchError((e){
			print("exception on setFont : ${e}");
		});
	}

	// if pos is not given, text may start with '\n' to specify to
	// draw text under last line
	void textTo(String text, [num x= null, num y= null, options= ""]) {
		if ((text == null || x == null) && options != "") {
			throw new Exception("options used only if text AND pos are specified");
		}
		_doCall("textTo", [text, x, y, options]);
	}

	void endPage() {
		_doCall("endPage");
	}

	void save() {
		_doCall("save");
	}

	void restore() {
		_doCall("restore");
	}

	void translate(num x, num y) {
		_doCall("translate", [x, y]);
	}

	void rotate(num angle) {
		_doCall("rotate", [angle]);
	}

	void setLineWidth(num width) {
		_doCall("setLineWidth", [width]);
	}

	void setColor(String kind, num r, num g, num b) {
		_doCall("setColor", [kind, r, g, b]);
	}

	void moveTo(num x, num y) {
		_doCall("moveTo", [x, y]);
	}

	void lineTo(num x, num y) {
		_doCall("lineTo", [x, y]);
	}

	void curveTo(num x1, num y1, num x2, num y2, num x3, num y3) {
		_doCall("curveTo", [x1, y1, x2, y2, x3, y3]);
	}

	Future loadImage(String imagePath, [ String options = "" ]) {
		return f.then((_) {
			return _servicePort.call(["loadImage", handle, imagePath, options]);
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
			return _servicePort.call(["imageTo", handle, imageHandle, x, y, options]);
		}).then((List result) {
			if (result[0] != null) {
				throw new Exception("failed : " + result[0]);
			}
		}).catchError((e){
			print("exception on imageTo : ${e}");
		});
	}

	void terminatePath(int what) {
		_doCall("terminatePath", [what]);
	}

	void _doCall(String command, [ List args = null ]) {
		f = f.then((_) {
//			print("calling ${command}");
			List callArgs = [ command, handle ];
			if (args != null) {
				callArgs.addAll(args);
			}
			return _servicePort.call(callArgs);
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
	SendPort _newServicePort() native "pdflibServicePort";
}

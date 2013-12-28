import 'dart:math';
import 'package:pdfdart/pdfdart.dart';

void main() {
    PDF pdf = new PDF("hello.pdf");
	if(PDF.getVersion() < 9) {
		pdf.setOption("textformat", "utf8");
	} else {
		pdf.setOption("stringformat", "utf8");
	}

	pdf.setInfo("Creator", "helloPdf.dart");
	pdf.setInfo("Author", "Christian Lefebvre");
	pdf.setInfo("Title", "Hello, Dart world !");

	pdf.beginPage(PDF.pageSize["a4"]); // ~ 600x800

	var font = pdf.loadFont("Helvetica-Bold");

	pdf.setFont(font, 24);
	pdf.textTo("Hello", 50, 600, "fillcolor={rgb 0.5 0.2 0.5}");
	pdf.textTo(", world !!");
	pdf.setFont(font, 20);
	pdf.textTo("\nsays Dart");
	pdf.setColor("fill", 0, 0, 1);
	pdf.textTo(" on ${new DateTime.now()}");

	pdf.setLineWidth(2.0);
	pdf.setColor("stroke", 1.0, 0.2, 0.2);
	pdf.setColor("fill", 0.0, 0.85, 0.85);
	pdf.textTo("\nsome text é, €, 漫畫画");
	pdf.textTo("\nsome stars ...");

	etoile(pdf, 100, 400, PDF.FILL);
	etoile(pdf, 300, 400, PDF.STROKE | PDF.CLOSE);
	etoile(pdf, 500, 400, PDF.FILL_STROKE);

	pdf..circle(100, 200, 50)
		..terminatePath(PDF.STROKE);
	pdf..circle(100, 200, 30)
		..terminatePath(PDF.FILL);

	pdf..arc(300, 200, 50, 30, 180)
		..terminatePath(PDF.FILL_STROKE);
	pdf..arc(300, 200, 50, 210, 0)
		..terminatePath(PDF.STROKE | PDF.CLOSE);

	num yy = sin(PI/3);
	var rosace = [
		[ -1, 0, -60, 60 ],
		[ 1, 0, 120, 240 ],
		[ -0.5, yy, -120, 0 ],
		[ 0.5, yy, 180, -60 ],
		[ -0.5, -yy, 0, 120 ],
		[ 0.5, -yy, 60, 180 ]
	];
	rosace.forEach((List<num> r) {
		pdf..arc(500 + r[0]*50, 200 + r[1]*50, 50, r[2], r[3])
			..terminatePath(PDF.STROKE);
	});

	var image1 = pdf.loadImage("chabenet.jpg");
	var image2 = pdf.loadImage("chabenet.png");

	pdf.imageTo(image1, 50, 650,
		"boxsize={200 150} position={center} fitmethod=meet showborder");
	pdf.imageTo(image2, 350, 650,
		"scale=0.22 boxsize={150 100} position={right top} fitmethod=clip showborder");

	pdf.endPage();

	pdf.close();
}

void etoile(PDF pdf, num x, num y, what) {
	pdf..save()
		..translate(x, y)
		..moveTo(0, 100)
		..lineTo(40, -80)
		..curveTo(60, -60, -55, 50, -75, 50)
		..curveTo(-100, 30, 100, 30, 75, 50)
		..curveTo(55, 50, -60, -60, -40, -80)
		..terminatePath(what)
		..restore();
}

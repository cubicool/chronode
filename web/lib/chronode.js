function deg2rad(d) { return d * (Math.PI / 180); }
function rad2deg(r) { return r * (180 / Math.PI); }

function arcxy(r, a) { return [
	Math.round(r * Math.cos(a)),
	Math.round(r * Math.sin(a))
]; }

// const x = { foo: 1, bar: 2, baz: 3 };
// console.log(...[x.foo, x.baz]);

/* class Canvas {
	constructor(canvas) {
		this.c = canvas.getContext("2d");
	}

	resize() {
		console.log("++ resize");

		this.w = this.c.clientWidth;
		this.h = this.c.clientHeight;

		this.draw();

		console.log("-- resize");
	};

	draw() {
		console.log("++ draw");
		console.log("cw = " + canvas.width + ", ch = " + canvas.height);
		console.log("ccw = " + canvas.clientWidth + ", cch = " + canvas.clientHeight);
		console.log("w = " + w + ", h = " + h);

		c.save();
		c.beginPath();
		c.strokeStyle = "#ffffff";
		c.lineWidth = 2;
		c.arc(w / 2, h / 2, w / 4, 0, 2 * Math.PI);
		c.stroke();
		c.restore();

		c.beginPath();
		// c.strokeStyle = "#ffffff";
		// c.lineWidth = 2;
		c.arc(w / 2, h / 2, w / 6, 0, 2 * Math.PI);
		c.stroke();

		console.log("-- draw");
	}

	// https://developer.mozilla.org/en-US/docs/Web/API/CanvasRenderingContext2D/save
	// strokeStyle, fillStyle, globalAlpha, lineWidth, lineCap, lineJoin, miterLimit,
	// lineDashOffset, shadowOffsetX, shadowOffsetY, shadowBlur, shadowColor,
	// globalCompositeOperation, font, textAlign, textBaseline, direction, imageSmoothingEnabled
	save() {}
	restore) {}
} */

// var canvas = document.getElementById("_canvas");
// var c = canvas.getContext("2d");

// https://developer.mozilla.org/en-US/docs/Web/API/CanvasRenderingContext2D/moveTo
const CRC2D_moveTo = CanvasRenderingContext2D.prototype.moveTo;

CanvasRenderingContext2D.prototype.moveTo = function(x, y) {
	this.pos = { x: x, y: y };

	CRC2D_moveTo.call(this, x, y);
}

CanvasRenderingContext2D.prototype.relMoveTo = function(x, y) {
	this.moveTo(this.pos.x + x, this.pos.y + y);
}

// https://developer.mozilla.org/en-US/docs/Web/API/CanvasRenderingContext2D/lineTo
const CRC2D_lineTo = CanvasRenderingContext2D.prototype.lineTo;

CanvasRenderingContext2D.prototype.lineTo = function(x, y) {
	this.pos = { x: x, y: y };

	CRC2D_lineTo.call(this, x, y);
}

CanvasRenderingContext2D.prototype.relLineTo = function(x, y) {
	this.lineTo(this.pos.x + x, this.pos.y + y);
}

// https://developer.mozilla.org/en-US/docs/Web/API/CanvasRenderingContext2D/arc
const CRC2D_arc = CanvasRenderingContext2D.prototype.arc;

// TODO: Take into account translation/scale/etc.
CanvasRenderingContext2D.prototype.arc = function(x, y, r, s, e, ccw=false) {
	const pos = arcxy(r, e);

	this.pos = { x: pos[0], y: pos[1] };

	CRC2D_arc.call(this, x, y, r, s, e, ccw);
}

/* CanvasRenderingContext2D.prototype.relArc = function(x, y) {
	this.arc(this.pos.x + x, this.pos.y + y);
} */

// canvas.width = canvas.height * (canvas.clientWidth / canvas.clientHeight);
// canvas.width = canvas.clientWidth
// canvas.height = canvas.clientHeight

// var w = 0; // canvas.width;
// var h = 0; // canvas.height;

function canvasGrid(gw, gh) {
	c.save();
	// c.scale(DPR, DPR);
	// c.translate(0.5, 0.5);

	// c.lineWidth = 3;
	// c.strokeStyle = "rgba(1, 1, 1, 1)";

	for(let x = gw; x < w; x += gw) {
		c.moveTo(x, 0);
		c.lineTo(x, h);
		c.stroke();
	}

	for(let y = gh; y < h; y += gh) {
		c.moveTo(0, y);
		c.lineTo(w, y);
		c.stroke();
	}

	c.restore();
}

function canvasScalePPI(width, height) {
	const DPR = window.devicePixelRatio;

	width = Math.ceil(width * DPR);
	height = Math.ceil(height * DPR);

	// Set the canvas resolution dimensions (integer values).
	canvas.width = width;
	canvas.height = height;

	// XXX: KEY STEP!
	// Set the canvas layout dimensions with respect to the canvas
	// resolution dimensions; note: not necessarily integer values!
	canvas.style.width = `${width / DPR}px`;
	canvas.style.height = `${height / DPR}px`;

	// Adjust canvas coordinates to use CSS pixel coordinates.
	const c = canvas.getContext("2d");

	console.log(`canvasScalePPI(${width}, ${height})`);

	c.scale(DPR, DPR);
}

function canvasResize(canvas) {
	console.log("++ resize");

	const w = canvas.clientWidth;
	const h = canvas.clientHeight;

	// canvas.width = Math.floor(w * DPR);
	// canvas.height = Math.floor(h * DPR);

	canvas.width = w;
	canvas.height = h;

	console.log(`w = ${w}, h = ${h}`);

	// canvasDraw();

	console.log("-- resize");

	return [w, h];
}

// console.log(arcxy(200, 0));
// console.log(arcxy(200, deg2rad(90)));
// console.log(arcxy(200, deg2rad(180)));
// console.log(arcxy(200, deg2rad(270)));

function canvasDraw() {
	// console.log("++ draw");
	// console.log("cw = " + canvas.width + ", ch = " + canvas.height);
	// console.log("ccw = " + canvas.clientWidth + ", cch = " + canvas.clientHeight);
	// console.log("w = " + w + ", h = " + h);
	const GoldenAngle = deg2rad(137.5);
	const piGAdiv2 = (Math.PI - GoldenAngle) / 2;
	const r = 200;

	c.strokeStyle = "#fff";
	c.lineWidth = 0.5;

	c.translate(0.5, 0.5);

	canvasGrid(12, 12);

	return;

	// TODO: To get the current translation is c.getTransform().{e,f}
	c.translate(101, 102);
	c.arc(w / 2, h / 2, 10, 0, 2 * Math.PI);
	c.fill();

	// c.moveTo(w / 2, h / 2);
	// c.relLineTo(100, 0);
	c.lineTo(100, 0);
	c.stroke();

	/*
	// Default settings, no translation.
	c.save();

	c.translate(w / 2, h / 2);

	// Draw the main circle outline.
	c.save(); {
		c.beginPath(); {
			c.arc(0, 0, 5, 0, 2 * Math.PI);
			c.fill();
		}

		c.beginPath(); {
			c.arc(0, 0, r, 0, 2 * Math.PI);
			c.stroke();
		}
	}; c.restore();

	// Draw the GoldenAngle outline.
	c.save(); {
		// c.lineWidth = 4;

		c.beginPath(); {
			c.rotate(-piGAdiv2);
			c.moveTo(0, 0);
			c.lineTo(r, 0);

			for(let i = 0; i < 3; i++) {
				c.rotate(-(GoldenAngle / 3));
				c.lineTo(r, 0);
			}

			c.lineTo(0, 0);
			c.stroke();
		}
	}; c.restore();

	// Back to default, non-translated.
	c.restore();

	// console.log("-- draw");
	*/
}

// https://www.w3schools.com/jsref/dom_obj_event.asp
// TODO: Why can't we call "canvas.addEventListener"?
// window.addEventListener("resize", canvasResize);

// canvasResize();
// canvasScalePPI(canvas.width, canvas.height);

function chronode(canvasName, node, rows) {
	let canvas = document.getElementById(canvasName);
	let c = canvas.getContext("2d");

	const [w, h] = canvasResize(canvas);

	// We treat the first Node as the "master", using its members as guides for the extents of
	// the visualization.
	const name = node.name;
	const start = node.start;
	const stop = node.stop;
	// const PPU = canvas.width / (stop - start);
	const PPU = w / (stop - start);

	const rowHeight = h / rows; // canvas.height / rows;

	console.log(`Recursing: ${node.name}`);
	console.log(`Range: start=${start}, stop=${stop}`);
	console.log(`Span: ${stop - start}ms`);
	// console.log(`PPU (Pixels Per Unit): ${canvas.width / (stop - start)}px`);
	console.log(`PPU (Pixels Per Unit): ${w / (stop - start)}px`);
	console.log(`Rows: ${rows}`);

	for(let i = 0; i < rows; i++) {
		console.log(`Drawing row ${i}...`);

		const yStart = i * rowHeight;
		const yEnd = yStart + rowHeight;

		c.fillStyle = `rgba(100, 150, 255, 1)`;
		c.fillRect(0, yStart, w, yEnd - yStart);

		const centerX = w / 2;
		const centerY = yStart + rowHeight / 2;

		/* c.beginPath();
		c.arc(centerX, centerY, 10, 0, 2 * Math.PI);
		c.fillStyle = "red";
		c.fill(); */

		c.beginPath();
		c.moveTo(0, yEnd);
		c.lineTo(w, yEnd);
		c.strokeStyle = "white";
		c.lineWidth = 2;
		c.stroke();
	}

	function _chronode(n, depth=0) {
		const y = (depth * rowHeight) + (rowHeight / 2);
		const x0 = Math.round((n.start - start) * PPU);
		const x1 = Math.round((n.stop - start) * PPU);

		console.log(`_chronde: ${n.name} @ ${depth}`);

		c.beginPath();
		c.moveTo(x0, y);
		c.lineTo(x1, y);
		c.strokeStyle = "red";
		c.lineWidth = 4;
		c.stroke();

		for(var x of [x0, x1]) {
			// console.log(`Doing: x=${x}, y=${y}`);

			c.beginPath();
			c.arc(x, y, 10, 0, 2 * Math.PI);
			c.fillStyle = "red";
			c.fill();
			c.closePath();
		}

		for(let i = 0; i < n.children.length; i++) {
			_chronode(n.children[i], depth + 1);
		}
	}

	_chronode(node.children[0], 0);

	// canvasScalePPI(canvas.width, canvas.height);

	/* function _chronode(node, depth=0) {
		// If the depth is 0, it's the "master" node we already queried above; so, do nothing
		// except handle the children.
		// if(depth) {
		if(true) {
			// console.log(` >> ${node.start - start} -> ${node.stop - start}`);

			const y = (depth + 1) * 100;
			const x0 = Math.round((node.start - start) * PPU);
			const x1 = Math.round((node.stop - start) * PPU);

			c.save();
			c.translate(0.5, 0.5);

			c.fillStyle = "#fff";
			c.strokeStyle = "#fff";
			c.lineWidth = 1;

			console.log(`${node.name} at ${depth} (${node.stop - node.start}ms) x0=${x0}, x1=${x1}`);

			for(var x of [x0, x1]) {
				// console.log(`Doing: x=${x}, y=${y}`);

				c.beginPath();
				c.arc(x, y, 2, 0, 2 * Math.PI);
				c.fill();
				c.closePath();
			}

			c.beginPath();
			c.moveTo(x0, y);
			c.lineTo(x1, y);
			c.stroke();
			c.closePath();

			// https://stackoverflow.com/questions/15661339/how-do-i-fix-blurry-text-in-my-html5-canvas
			c.textAlign = "center";
			c.font = "11px Segoe";

			c.save();
			c.translate(0.5, 0.5);

			c.fillStyle = "#666";

			c.fillText(node.name, x0 + Math.round(((x1 - x0) / 2)), y - 5);
			c.fillText(`${node.stop - node.start}ms`, x0 + Math.round(((x1 - x0) / 2)), y + 14);

			c.restore();

			c.fillText(node.name, x0 + Math.round(((x1 - x0) / 2)), y - 5);
			c.fillText(`${node.stop - node.start}ms`, x0 + Math.round(((x1 - x0) / 2)), y + 14);

			c.restore();
		}

		for(var i = 0; i < node.children.length; i++) {
			_chronode(node.children[i], depth + 1);
		}
	} */

	/* c.save();
	c.fillStyle = "#999";
	c.fillRect(0, 0, canvas.width, canvas.height);
	c.restore();

	_chronode(node); */
}

console.log("sketching bro");

function setup() {
  createCanvas(800, 800);
}

function draw() {
  background(220);
  // console.log(forceValue);
  let r = map(forceValue, 0, 255, width, width/10);
  ellipse(width/2, height/2, r, r);
}

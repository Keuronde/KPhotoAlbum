imageshow_tpl=`

<div id="imageContainer">
{{#image}}
<img src="{{url}}" id="image">
{{/image}}

<div id="toPrevImage" onclick="lfg_goNext()"></div>
<div id="toNextImage" onclick="lfg_goPrevious()"></div>
<img src="close.png" id="closeButton" onclick="lfg_close()">
</div>
`

function lfg_init(){
	links = [].slice.call(document.querySelectorAll("[rel^=lightbox]"));
	links.forEach(
	function(element){
		element.addEventListener("click", lfg_start, false);
	})
}

function lfg_start(event){

	event.preventDefault();
	event.stopPropagation();
	
	// Backup scroll
	scrollPosition = window.pageYOffset;
	console.log(scrollPosition)
	
	// find all images in the gallery
	var galleryName = this.getAttribute("data-lightbox");
	galleryNode = [].slice.call(document.querySelectorAll("[data-lightbox^="+galleryName+"]"));
	gallery=[];
	imagePositionInGallery=0;
	imageHref = this.getAttribute('href');
	
	// Build gallery array and find current position
	galleryNode.forEach(
	function(element){
		gallery.push({
			href: element.getAttribute('href')
		})
		if(element.getAttribute('href') === imageHref){
			imagePositionInGallery = gallery.length - 1;
		}
	}
	)
	// display the image
	lfg_render({"image":{"url":this.href}});
	// Hide everything else
	document.getElementById('lfg_hidable').style["display"]="none";

	// Manage Keyboard
	window.addEventListener("keydown", lfg_keyboardNav , true);
	// Manage Swipe
	var touchstartX = 0;
	var touchstartY = 0;
	var touchendX = 0;
	var touchendY = 0;
	const gestureZone = document.getElementById('imageContainer');

	gestureZone.addEventListener('touchstart', function(event) {
		  touchstartX = event.changedTouches[0].screenX;
		  touchstartY = event.changedTouches[0].screenY;
		  console.log('swipe start');
	}, false);

	gestureZone.addEventListener('touchend', function(event) {
		  touchendX = event.changedTouches[0].screenX;
		  touchendY = event.changedTouches[0].screenY;
		  console.log('swipe end');
		  handleGesture();
	}, false); 


	return false;
}

function lfg_render(data){
	// display the image
	Mustache.parse(imageshow_tpl);
    //Render the data into the template
	var rendered = Mustache.render(imageshow_tpl, data);
	document.getElementById('photoFullscreen').innerHTML = rendered;
}

function lfg_goNext(){
	if(imagePositionInGallery < (gallery.length - 1)){
		data = {"image":{"url":gallery[++imagePositionInGallery].href}}
	}
	lfg_render(data);
}

function lfg_goPrevious(){
	if(imagePositionInGallery > 0){
		data = {"image":{"url":gallery[--imagePositionInGallery].href}}
	}
	lfg_render(data);
}

function lfg_close(){
	window.removeEventListener("keydown", lfg_keyboardNav , true);
	document.getElementById('photoFullscreen').innerHTML = "";
	document.getElementById('lfg_hidable').style["display"]="block";
	window.scrollTo(0,scrollPosition);
}


// Keyboard management
function lfg_keyboardNav(event) {
	if (event.defaultPrevented) {
		//return; // Should do nothing if the key event was already consumed.
	}

	switch (event.key) {
		case "ArrowRight":
		case "ArrowDown":
		case "Enter":
		case "Spacebar":
		case " ":
		  lfg_goNext();
		  break;
		case "ArrowUp":
		case "ArrowLeft":
		  lfg_goPrevious();
		  break;
		case "Escape":
		case "Esc":
		  lfg_close();
		  break;
		default:
		  return; // Quit when this doesn't handle the key event.
	}

	// Consume the event for suppressing "double action".
	event.preventDefault();
}


// Swipe management





function handleGesture() {
    if (touchendX <= touchstartX) {
        console.log('Swiped left');
    }
    
    if (touchendX >= touchstartX) {
        console.log('Swiped right');
    }
    
    if (touchendY <= touchstartY) {
        console.log('Swiped up');
    }
    
    if (touchendY >= touchstartY) {
        console.log('Swiped down');
    }
    
    if (touchendY === touchstartY) {
        console.log('Tap');
    }
    lfg_close()
}


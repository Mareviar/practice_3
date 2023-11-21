function copyLink() {
  const button = document.getElementById("Link");
  const buttonText = button.textContent;

  const tempInput = document.createElement("input");
  tempInput.value = buttonText;

  document.body.appendChild(tempInput);

  tempInput.select();

  try {
    document.execCommand("copy");
  } catch (err) {
    console.error("Не удалось скопировать текст: ", err);
  }
  document.body.removeChild(tempInput);
}

function create_link() {
  const linkInput = document.getElementById('linkInput');
  const inputValue = linkInput.value;

  // Get the CSRF token from the cookie
  var csrftoken = getCookie('csrftoken');

  fetch('/create_link/', {
      method: 'POST',
      body: `inputValue=${inputValue}`,
      headers: {
          'Content-Type': 'application/x-www-form-urlencoded',
          'X-CSRFToken': csrftoken, // Include the CSRF token in the headers
      },
  })
  .then(function (response) {
      return response.text();
  })
  .then(function (data) {
      const container = document.querySelector(".Input");
      if (data.includes("10.241.125.222")) {
        const testLinksElement = document.getElementById('Link');
        if (testLinksElement){
          testLinksElement.remove();
        }
        var linkElement = document.createElement('button');
          linkElement.className = 'Link';
          linkElement.id = 'Link';
          linkElement.textContent = data;
          linkElement.addEventListener('click', copyLink);
          container.appendChild(linkElement)
      }
  })
  .catch(function (error) {
      console.error('Error:', error);
  });

  // Prevent the default form submission behavior
  event.preventDefault();
}

// Function to get the CSRF token from the cookie
function getCookie(name) {
  var cookieValue = null;
  if (document.cookie && document.cookie !== '') {
      var cookies = document.cookie.split(';');
      for (var i = 0; i < cookies.length; i++) {
          var cookie = cookies[i].trim();
          if (cookie.substring(0, name.length + 1) === name + '=') {
              cookieValue = decodeURIComponent(cookie.substring(name.length + 1));
              break;
          }
      }
  }
  return cookieValue;
}
# hair-filter-sandbox
Sandbox project for a hair-filter application.

We can add hair filters to images of people by rendering 3D hair models in the image and placing it on the person's head.

<p float="left" align="center">
  <img src="https://user-images.githubusercontent.com/19911240/163695936-de061e7c-9648-45a2-89da-212bf478d3db.jpg" width="200" />
  &rarr;
  <img src="https://user-images.githubusercontent.com/19911240/163695972-afbc2ca6-7861-452a-88d7-b1f1015c4c72.png" width="200" /> 
</p>

The image and 3D hair model is rendered using OpenGL.

The image is processed through a face cascade model and shape predictor model to detect a person's face and get face landmark points. We also detect the face pose in the image to determine the angle position of the hair model.

The face landmark points are mapped to a pre-configured point in the hair model. This allows the hair model to be dynamically placed on the persons head.

An outline of the peron's face and neck is resized, cropped and displayed a distance away from the original image. The 3D hair model is placed on the outline. This allows the hair to cover parts of the image that does not belong to the face.

<p float="left" align="center">
  <img src="https://user-images.githubusercontent.com/19911240/163696689-6fa7c12a-e533-47b1-b8b3-adc9cd9bd3fe.png" width="200" />
</p>

Written in C++, it uses OpenGL, OpenCV and Dlib libraries.
